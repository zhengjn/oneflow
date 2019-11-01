#include "oneflow/xrt/xla/xla_executable.h"
#include "oneflow/xrt/xla/xla_executable_context.h"
#include "oneflow/xrt/xla/xla_executable_scope.h"
#include "oneflow/xrt/xla/xla_macro.h"
#include "oneflow/xrt/xla/xla_resource_manager.h"

namespace oneflow {
namespace xrt {
namespace mola {

bool XlaExecutable::Run(const std::vector<Parameter> &inputs,
                        const ExecutableRunOptions &run_options,
                        bool block_until_done) {
  CHECK_EQ(inputs.size(), input_shapes_.size())
      << "Size mismatch between input params and input shapes.";
  XlaExecutableRunContext run_context(run_options, device_);
  // Translate inputs to ShapedBuffer for suitable running the executable.
  const auto &input_buffers =                             // NOLINT
      run_context.PopulateInputs(inputs, input_shapes_);  // NOLINT

  // Populate output params to reuse the buffers in allocator. This helps
  // to reduce memory occupancy and avoid extra copy between temporary
  // buffers and output buffers.
  const auto &return_params = run_options.return_params;
  run_context.PopulateResultBuffers(return_params, executable_.get());

  MOLA_CHECK_AND_ASSIGN(auto run_result, [&]() {
    XlaExecutableRunScope scope(executable_.get(), run_context);

    xla::ExecutableRunOptions options;
    options.set_stream(run_context.stream());
    options.set_allocator(run_context.allocator());
    options.set_intra_op_thread_pool(run_context.host_device());
    options.set_rng_seed(run_context.rng_seed());

    auto result = executable_->RunAsync(input_buffers, options);
    if (block_until_done) {
      run_context.stream()->BlockHostUntilDone();
    }
    return std::move(result);
  }());

  // Result shape should be tuple
  CHECK(run_result.on_host_shape().IsTuple());

  // Translate result to output blobs
  for (int i = 0; i < return_params.size(); ++i) {
    se::DeviceMemoryBase buffer = run_result.buffer({i});
    if (buffer.opaque()) {
      CHECK_EQ(buffer.opaque(), return_params[i].data());
    }
  }

  this->results_ = std::move(return_params);
  return true /*Success*/;
}

}  // namespace mola
}  // namespace xrt
}  // namespace oneflow

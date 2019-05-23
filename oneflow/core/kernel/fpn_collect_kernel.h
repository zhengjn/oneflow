#ifndef ONEFLOW_CORE_KERNEL_FPN_COLLECT_KERNEL_H_
#define ONEFLOW_CORE_KERNEL_FPN_COLLECT_KERNEL_H_

#include "oneflow/core/kernel/kernel.h"
#include "oneflow/core/kernel/bbox_util.h"

namespace oneflow {

template<DeviceType device_type, typename T>
class FpnCollectKernel final : public KernelIf<device_type> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(FpnCollectKernel);
  FpnCollectKernel() = default;
  ~FpnCollectKernel() = default;

  using BBox = IndexedBBoxT<const T>;
  using MutBBox = IndexedBBoxT<T>;

 private:
  void ForwardDataContent(const KernelCtx&,
                          std::function<Blob*(const std::string&)>) const override;
  void ForwardDim0ValidNum(const KernelCtx& ctx,
                           std::function<Blob*(const std::string&)> BnInOp2Blob) const override;
  void ForwardRecordIdInDevicePiece(
      const KernelCtx& ctx, std::function<Blob*(const std::string&)> BnInOp2Blob) const override;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_KERNEL_RELU_KERNEL_H_

#define private public
#include "oneflow/core/vm/control_stream_type.h"
#include "oneflow/core/common/util.h"
#include "oneflow/core/vm/vm.msg.h"
#include "oneflow/core/vm/vm_desc.msg.h"
#include "oneflow/core/vm/vm_util.h"
#include "oneflow/core/vm/test_util.h"
#include "oneflow/core/vm/stream_type.h"
#include "oneflow/core/vm/instruction_type.h"
#include "oneflow/core/vm/test_util.h"
#include "oneflow/core/common/cached_object_msg_allocator.h"

namespace oneflow {
namespace vm {

namespace test {

namespace {

using InstructionMsgList = OBJECT_MSG_LIST(InstructionMsg, instr_msg_link);

TEST(ControlStreamType, new_symbol_symbol) {
  auto vm_desc = ObjectMsgPtr<VmDesc>::New(TestUtil::NewVmResourceDesc().Get());
  TestUtil::AddStreamDescByInstrNames(vm_desc.Mutable(), {"NewConstHostSymbol"});
  CachedObjectMsgAllocator allocator(20, 100);
  auto vm = ObjectMsgPtr<VirtualMachine>::NewFrom(&allocator, vm_desc.Get());
  InstructionMsgList list;
  int64_t symbol_id = NewConstHostLogicalObjectId();
  list.EmplaceBack(NewInstruction("NewConstHostSymbol")->add_int64_operand(symbol_id));
  ASSERT_TRUE(vm->pending_msg_list().empty());
  vm->Receive(&list);
  ASSERT_EQ(vm->pending_msg_list().size(), 1 * 2);
  vm->Schedule();
  ASSERT_TRUE(vm->pending_msg_list().empty());
  ASSERT_TRUE(vm->waiting_instruction_list().empty());
  ASSERT_TRUE(vm->active_stream_list().empty());
  ASSERT_EQ(vm->thread_ctx_list().size(), 1 * 2);
  ASSERT_EQ(vm->stream_type_id2stream_rt_desc().size(), 1 * 2);
  ASSERT_EQ(vm->id2logical_object().size(), 1 * 2);
  auto* logical_object = vm->mut_id2logical_object()->FindPtr(symbol_id);
  ASSERT_NE(logical_object, nullptr);
  ASSERT_EQ(logical_object->global_device_id2mirrored_object().size(), 1);
  ASSERT_TRUE(vm->Empty());
}

}  // namespace

}  // namespace test

}  // namespace vm
}  // namespace oneflow

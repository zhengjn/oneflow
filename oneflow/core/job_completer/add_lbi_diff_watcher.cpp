/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "oneflow/core/job_completer/op_graph_pass.h"
#include "oneflow/core/job/lbi_diff_watcher_info.pb.h"
#include "oneflow/core/operator/operator.h"

namespace oneflow {

namespace {

class AddLbiDiffWatcherOpConfs final : public OpGraphPass {
 public:
  bool IsEnabled() const override { return GlobalJobDesc().IsTrain(); }
  Maybe<void> Apply(Job* job) const override;
};

Maybe<void> AddLbiDiffWatcherOpConfs::Apply(Job* job) const {
  JobBuilder job_builder(job);
  const auto& map = Global<LbiDiffWatcherInfo>::Get()->job_name2lbi_and_watcher_uuids();
  if (map.find(GlobalJobDesc().job_name()) == map.end()) { return Maybe<void>::Ok(); }
  const auto& tag2lbi_relations = job->helper().tag2lbi_relations();
  const auto& conf_iter = tag2lbi_relations.find(kProducedLbi2ConsumedDiffLbi);
  if (conf_iter == tag2lbi_relations.end()) { return Maybe<void>::Ok(); }
  HashMap<LogicalBlobId, LogicalBlobId> lbi2diff_lbi;
  for (const auto& pair : conf_iter->second.pair()) {
    CHECK(lbi2diff_lbi.emplace(pair.first(), pair.second()).second);
  }
  const auto& pair_list = map.at(GlobalJobDesc().job_name()).lbi_and_uuid_pair();
  for (const LbiAndDiffWatcherUuidPair& pair : pair_list) {
    if (lbi2diff_lbi.find(pair.lbi()) == lbi2diff_lbi.end()) { continue; }
    OperatorConf foreign_watcher_op;
    foreign_watcher_op.set_name("System-LbiDiffWatcher-ForeignWatcher-" + NewUniqueId());
    auto* foreign_watcher_conf = foreign_watcher_op.mutable_foreign_watch_conf();
    foreign_watcher_conf->set_in(GenLogicalBlobName(lbi2diff_lbi.at(pair.lbi())));
    foreign_watcher_conf->set_handler_uuid(pair.watcher_uuid());
    job_builder.AddOps(job_builder.ParallelConf4Lbi(pair.lbi()), {foreign_watcher_op});
  }
  return Maybe<void>::Ok();
}

REGISTER_FUNCTION_PASS("AddLbiDiffWatcherOpConfs", AddLbiDiffWatcherOpConfs);

}  // namespace

}  // namespace oneflow

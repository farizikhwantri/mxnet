/*!
 *  Copyright (c) 2016 by Contributors
 * \file broadcast_reduce_op.cc
 * \brief CPU Implementation of broadcast and reduce functions.
 */
#include "./broadcast_reduce_op.h"

namespace mxnet {
namespace op {
MXNET_OPERATOR_REGISTER_REDUCE_AXIS(argmax)
.describe(R"code(Returns indices of the maximum values along an axis.  

In the case of multiple occurrences of maximum values, the indices corresponding to the first occurrence
are returned.

Examples::  

  x = [[ 0.,  1.,  2.], 
       [ 3.,  4.,  5.]]  

  // argmax along axis 0
  argmax(x, axis=0) = [ 1.,  1.,  1.]   

  // argmax along axis 1
  argmax(x, axis=1) = [ 2.,  2.]   

  // argmax along axis 1 keeping same dims as an input array
  argmax(x, axis=1, keepdims=True) = [[ 2.], 
                                      [ 2.]]   

)code" ADD_FILELINE)
.set_attr<FCompute>("FCompute<cpu>", SearchAxisCompute<cpu, mshadow::red::maximum>)
.set_attr<nnvm::FGradient>("FGradient", MakeZeroGradNodes);

MXNET_OPERATOR_REGISTER_REDUCE_AXIS(argmin)
.describe(R"code(Returns indices of the minimum values along an axis.

In the case of multiple occurrences of minimum values, the indices corresponding to the first occurrence
are returned.

Examples::  

  x = [[ 0.,  1.,  2.], 
       [ 3.,  4.,  5.]]  

  // argmin along axis 0
  argmin(x, axis=0) = [ 0.,  0.,  0.]   

  // argmin along axis 1
  argmin(x, axis=1) = [ 0.,  0.]   

  // argmin along axis 1 keeping same dims as an input array
  argmin(x, axis=1, keepdims=True) = [[ 0.], 
                                      [ 0.]]   

)code" ADD_FILELINE)
.set_attr<FCompute>("FCompute<cpu>", SearchAxisCompute<cpu, mshadow::red::minimum>)
.set_attr<nnvm::FGradient>("FGradient", MakeZeroGradNodes);

// Legacy support
NNVM_REGISTER_OP(argmax_channel)
.describe(R"code(Returns argmax indices of each channel from the input array.

The result will be an NDArray of shape (num_channel,).

In case of multiple occurrences of the maximum values, the indices corresponding to the first occurrence
are returned.

Examples::  

  x = [[ 0.,  1.,  2.], 
       [ 3.,  4.,  5.]]  

  argmax_channel(x) = [ 2.,  2.]   

)code" ADD_FILELINE)
.set_num_inputs(1)
.set_num_outputs(1)
.set_attr_parser([](NodeAttrs* attrs) {
    ReduceAxisParam param;
    param.axis = 1;
    param.keepdims = false;
    attrs->parsed = param;
  })
.set_attr<nnvm::FInferShape>("FInferShape", ReduceAxisShape)
.set_attr<nnvm::FInferType>("FInferType", ElemwiseType<1, 1>)
.set_attr<FCompute>("FCompute<cpu>", SearchAxisCompute<cpu, mshadow::red::maximum>)
.add_argument("data", "NDArray-or-Symbol", "The input array");

NNVM_REGISTER_OP(pick)
.describe(R"code(Picks elements from an input array according to the input indices along the given axis.

Given an input array of shape ``(d0, d1)`` and indices of shape ``(i0,)``, the result will be
an output array of shape ``(i0,)`` with::

  output[i] = input[i, indices[i]]

By default, if any index mentioned is too large, it is replaced by the index that addresses
the last element along an axis.

This function supports n-dimensional input and (n-1)-dimensional indices arrays.

Examples::

  x = [[ 1.,  2.],
       [ 3.,  4.],
       [ 5.,  6.]]

  // picks elements with specified indices along axis 0
  pick(x, y=[0,1], 0) = [ 1.,  4.]

  // picks elements with specified indices along axis 1
  pick(x, y=[0,1,0], 1) = [ 1.,  4.,  5.]

  y = [[ 1.],
       [ 0.],
       [ 2.]]

  // picks elements with specified indices along axis 1 and dims are maintained
  pick(x,y, 1, keepdims=True) = [[ 2.],
                                 [ 3.],
                                 [ 6.]]

)code" ADD_FILELINE)
.set_num_inputs(2)
.set_num_outputs(1)
.set_attr_parser(ParamParser<ReduceAxisParam>)
.set_attr<nnvm::FListInputNames>("FListInputNames",
  [](const NodeAttrs& attrs) {
    return std::vector<std::string>{"data", "index"};
  })
.set_attr<nnvm::FInferShape>("FInferShape", PickOpShape)
.set_attr<nnvm::FInferType>("FInferType", PickOpType)
.set_attr<FCompute>("FCompute<cpu>", PickOpForward<cpu>)
.set_attr<nnvm::FGradient>("FGradient",
  [](const nnvm::NodePtr& n, const std::vector<nnvm::NodeEntry>& ograds) {
    auto ret = MakeNonlossGradNode("_backward_pick", n, ograds,
                                   {n->inputs[1]}, n->attrs.dict);
    auto p = MakeNode("zeros_like", n->attrs.name + "_index_backward",
                      {n->inputs[1]}, nullptr, &n);
    ret.emplace_back(nnvm::NodeEntry{p, 0, 0});
    return ret;
  })
.add_argument("data", "NDArray-or-Symbol", "The input array")
.add_argument("index", "NDArray-or-Symbol", "The index array")
.add_arguments(ReduceAxisParam::__FIELDS__());


NNVM_REGISTER_OP(_backward_pick)
.set_num_inputs(2)
.set_num_outputs(1)
.set_attr_parser(ParamParser<ReduceAxisParam>)
.set_attr<nnvm::TIsBackward>("TIsBackward", true)
.set_attr<FCompute>("FCompute<cpu>", PickOpBackward<cpu>);

}  // namespace op
}  // namespace mxnet

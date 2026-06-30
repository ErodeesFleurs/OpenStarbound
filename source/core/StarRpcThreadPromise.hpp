#pragma once

#include "StarRpcPromise.hpp"

namespace Star {

template <typename Result, typename Error = String>
using RpcThreadPromise = RpcPromise<Result, Error, true>;

template <typename Result, typename Error = String>
using RpcThreadPromiseKeeper = RpcPromiseKeeper<Result, Error, true>;

}

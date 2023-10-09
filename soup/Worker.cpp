#include "Worker.hpp"

#include "Promise.hpp"
#include "Socket.hpp"

namespace soup
{
	void Worker::awaitPromiseCompletion(Promise<void>* p, void(*f)(Worker&, Capture&&), Capture&& cap)
	{
		if (!p->isPending() && canRecurse())
		{
			f(*this, std::move(cap));
		}
		else
		{
			holdup_type = PROMISE_VOID;
			holdup_callback.set(f, std::move(cap));
			holdup_data = p;
		}
	}

	std::string Worker::toString() const
	{
#if !SOUP_WASM
		if (type == WORKER_TYPE_SOCKET)
		{
			return static_cast<const Socket*>(this)->toString();
		}
#endif
		return "[Worker]";
	}
}

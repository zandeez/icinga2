/* Icinga 2 | (c) 2012 Icinga GmbH | GPLv2+ */

#include "remote/eventqueue.hpp"
#include "remote/filterutility.hpp"
#include "base/io-engine.hpp"
#include "base/singleton.hpp"
#include "base/logger.hpp"
#include "base/utility.hpp"
#include <boost/asio/spawn.hpp>

using namespace icinga;

EventQueue::EventQueue(String name)
	: m_Name(std::move(name))
{ }

bool EventQueue::CanProcessEvent(const String& type) const
{
	boost::mutex::scoped_lock lock(m_Mutex);

	return m_Types.find(type) != m_Types.end();
}

void EventQueue::ProcessEvent(const Dictionary::Ptr& event)
{
	Namespace::Ptr frameNS = new Namespace();
	ScriptFrame frame(true, frameNS);
	frame.Sandboxed = true;

	try {
		if (!FilterUtility::EvaluateFilter(frame, m_Filter.get(), event, "event"))
			return;
	} catch (const std::exception& ex) {
		Log(LogWarning, "EventQueue")
			<< "Error occurred while evaluating event filter for queue '" << m_Name << "': " << DiagnosticInformation(ex);
		return;
	}

	boost::mutex::scoped_lock lock(m_Mutex);

	typedef std::pair<void *const, std::deque<Dictionary::Ptr> > kv_pair;
	for (kv_pair& kv : m_Events) {
		kv.second.push_back(event);
	}

	m_CV.notify_all();
}

void EventQueue::AddClient(void *client)
{
	boost::mutex::scoped_lock lock(m_Mutex);

	auto result = m_Events.insert(std::make_pair(client, std::deque<Dictionary::Ptr>()));
	ASSERT(result.second);

#ifndef I2_DEBUG
	(void)result;
#endif /* I2_DEBUG */
}

void EventQueue::RemoveClient(void *client)
{
	boost::mutex::scoped_lock lock(m_Mutex);

	m_Events.erase(client);
}

void EventQueue::UnregisterIfUnused(const String& name, const EventQueue::Ptr& queue)
{
	boost::mutex::scoped_lock lock(queue->m_Mutex);

	if (queue->m_Events.empty())
		Unregister(name);
}

void EventQueue::SetTypes(const std::set<String>& types)
{
	boost::mutex::scoped_lock lock(m_Mutex);
	m_Types = types;
}

void EventQueue::SetFilter(std::unique_ptr<Expression> filter)
{
	boost::mutex::scoped_lock lock(m_Mutex);
	m_Filter.swap(filter);
}

Dictionary::Ptr EventQueue::WaitForEvent(void *client, double timeout)
{
	boost::mutex::scoped_lock lock(m_Mutex);

	for (;;) {
		auto it = m_Events.find(client);
		ASSERT(it != m_Events.end());

		if (!it->second.empty()) {
			Dictionary::Ptr result = *it->second.begin();
			it->second.pop_front();
			return result;
		}

		if (!m_CV.timed_wait(lock, boost::posix_time::milliseconds(long(timeout * 1000))))
			return nullptr;
	}
}

Dictionary::Ptr EventQueue::WaitForEvent(void *client, boost::asio::yield_context yc, double timeout)
{
	double deadline = -1.0;

	for (;;) {
		{
			boost::mutex::scoped_try_lock lock(m_Mutex);

			if (lock.owns_lock()) {
				auto it = m_Events.find(client);
				ASSERT(it != m_Events.end());

				if (it->second.empty()) {
					if (deadline == -1.0) {
						deadline = Utility::GetTime() + timeout;
					} else if (Utility::GetTime() >= deadline) {
						return nullptr;
					}
				} else {
					Dictionary::Ptr result = *it->second.begin();
					it->second.pop_front();
					return result;
				}
			}
		}

		IoBoundWorkSlot dontLockTheIoThreadWhileWaiting (yc);
	}
}

std::vector<EventQueue::Ptr> EventQueue::GetQueuesForType(const String& type)
{
	EventQueueRegistry::ItemMap queues = EventQueueRegistry::GetInstance()->GetItems();

	std::vector<EventQueue::Ptr> availQueues;

	typedef std::pair<String, EventQueue::Ptr> kv_pair;
	for (const kv_pair& kv : queues) {
		if (kv.second->CanProcessEvent(type))
			availQueues.push_back(kv.second);
	}

	return availQueues;
}

EventQueue::Ptr EventQueue::GetByName(const String& name)
{
	return EventQueueRegistry::GetInstance()->GetItem(name);
}

void EventQueue::Register(const String& name, const EventQueue::Ptr& function)
{
	EventQueueRegistry::GetInstance()->Register(name, function);
}

void EventQueue::Unregister(const String& name)
{
	EventQueueRegistry::GetInstance()->Unregister(name);
}

EventQueueRegistry *EventQueueRegistry::GetInstance()
{
	return Singleton<EventQueueRegistry>::GetInstance();
}

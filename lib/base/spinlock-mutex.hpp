/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2018 Icinga Development Team (https://www.icinga.com/)  *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#ifndef SPINLOCK_MUTEX_H
#define SPINLOCK_MUTEX_H

#include <atomic>

namespace icinga
{

/**
 * A mutex based on spin locking
 *
 * The implementation details are explicitly inlined
 * as they're quite tiny (lines of code) and being executed often.
 *
 * @ingroup base
 */
class SpinlockMutex
{
public:
	inline SpinlockMutex(): m_IsLocked(ATOMIC_FLAG_INIT)
	{
	}

	SpinlockMutex(const SpinlockMutex&) = delete;
	SpinlockMutex(SpinlockMutex&&) = delete;

	inline ~SpinlockMutex()
	{
	}

	SpinlockMutex& operator= (const SpinlockMutex&) = delete;
	SpinlockMutex& operator= (SpinlockMutex&&) = delete;

	inline void lock()
	{
		while (m_IsLocked.test_and_set(std::memory_order_acquire))
			;
	}

	inline void unlock()
	{
		m_IsLocked.clear(std::memory_order_release);
	}

	inline bool try_lock()
	{
		return !m_IsLocked.test_and_set(std::memory_order_acquire);
	}

private:
	std::atomic_flag m_IsLocked;
};

}

#endif /* SPINLOCK_MUTEX_H */

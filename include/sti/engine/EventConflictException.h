/*! \file EventConflictException.h
 *  \author Jason Michael Hogan
 *  \brief Include-file for the class EventConflictException
 *  \section license License
 *
 *  Copyright (C) 2008 Jason Hogan <hogan@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
 *
 *  The STI is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The STI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the STI.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STI_ENGINE_EVENTCONFLICTEXCEPTION_H
#define STI_ENGINE_EVENTCONFLICTEXCEPTION_H

#include <sti/engine/RawEvent.h>
#include <sti/engine/STI_Exception.h>

#include <string>

namespace STI
{
namespace Engine
{

class EventConflictException : public STI_Exception
{
public:

	EventConflictException(const STI::Engine::RawEvent& evt, const std::string& message)
		: STI_Exception(message), _event1(evt), _event2(evt) {}
	EventConflictException(const STI::Engine::RawEvent& event1, const STI::Engine::RawEvent& event2, const std::string& message)
		: STI_Exception(message), _event1(event1), _event2(event2) {}
	~EventConflictException() throw() {}

	double lastTime() const
	{
		if (_event1.time() > _event2.time())
			return _event1.time();
		else
			return _event2.time();
	}

	const STI::Engine::RawEvent& getEvent1() const { return _event1; }
	const STI::Engine::RawEvent& getEvent2() const { return _event2; }

private:

	STI::Engine::RawEvent _event1;
	STI::Engine::RawEvent _event2;

};


} //Engine
} //STI


#endif


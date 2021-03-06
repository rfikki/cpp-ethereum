/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Interface.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Interface.h"

#include <libdevcore/Log.h>
#include <libp2p/All.h>
#include "WhisperHost.h"
using namespace std;
using namespace dev;
using namespace dev::p2p;
using namespace dev::shh;

#if defined(clogS)
#undef clogS
#endif
#define clogS(X) dev::LogOutputStream<X, true>(false) << "| " << std::setw(2) << session()->socketId() << "] "

bool MessageFilter::matches(Message const& _m) const
{
	for (auto const& t: m_topicMasks)
	{
		if (t.first.size() != t.second.size() || _m.topic.size() < t.first.size())
			continue;
		for (unsigned i = 0; i < t.first.size(); ++i)
			if (((t.first[i] ^ _m.topic[i]) & t.second[i]) != 0)
				goto NEXT;
		return true;
		NEXT:;
	}
	return false;
}

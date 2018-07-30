/*
 * Automata
 *
 * Copyright (c) 2018 Jose C. Rubio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __FSM_HPP_INCLUDE__
#define __FSM_HPP_INCLUDE__

#include <memory>
#include <map>
#include <functional>
#include <unordered_map>
#include <vector>
#include <assert.h>

namespace fsm {

struct CallbackKey
{
	uint32_t first;
	uint32_t second;
	bool operator==(const CallbackKey &other) const
		{ return (first == other.first
				  && second == other.second);
		}		
};
	
}
namespace std {
	template <>
    struct hash<fsm::CallbackKey>
    {
        size_t operator()( const fsm::CallbackKey& k ) const
		{		
			return (hash<uint32_t>()( k.first ) ^ (hash<uint32_t>()( k.second ) << 1)) >> 1;
		}
    };
}

namespace fsm {

class FSM;

template< class Dummy >
class BaseEvent_Statics
{
public:
	typedef uint32_t Family;
	static Family _family_counter;
};

template< class Dummy >
typename BaseEvent_Statics<Dummy>::Family BaseEvent_Statics<Dummy>::_family_counter = 0;
	
class BaseEvent : public BaseEvent_Statics<void>{
 public:
  virtual ~BaseEvent() {};
};

template <typename Derived>
class Event : public BaseEvent {
public:
  static Family family() {
  static Family family = _family_counter++;
    return family;
  }
};	

template <typename Dummy>	
class BaseState_Statics {
	
public:
	typedef uint32_t Index;
	static Index state_index_counter;	
};

template< class Dummy >
typename BaseState_Statics<Dummy>::Index BaseState_Statics<Dummy>::state_index_counter = 0;	
	
class BaseState : public BaseState_Statics<void> {
public:

	template <typename S>
	void transit();
	FSM* fsm;
};
	
class FSM {

private:
	// Functor used as an event signal callback that casts to E.
	struct EventCallbackBase {
		virtual void operator()(const void *event) = 0;
	};
	
	template <typename E>
	struct EventCallbackWrapper : public EventCallbackBase {
		explicit EventCallbackWrapper(std::function<void(const E &)> callback) : callback(callback) {}
		void operator()(const void *event) { callback(*(static_cast<const E*>(event))); }
		std::function<void(const E &)> callback;
	};
		
public:
	
	using state_ptr_t = std::shared_ptr<BaseState>;
	template<typename State, typename Event>
	void subscribe() {
		State *state_ptr = static_cast<State*>(_state_map[State::stateIndex()].get());		
		void (State::*react)(const Event &) = &State::react;
		auto wrapper = std::make_shared<EventCallbackWrapper<Event>>
			(std::bind(react, state_ptr, std::placeholders::_1));
		_callback_pool[CallbackKey{Event::family(), State::stateIndex()}].push_back(wrapper);	
	}

	template <typename E>
	void dispatch(const E& event) {
		CallbackKey key{E::family(), current_state_idx};
		for (auto& callback : _callback_pool[key]) {
			callback->operator()(&event);
		}		
	}
	
	template<typename S, typename ...Args>
	void addState(Args...args) {
		auto state_ptr = std::make_shared<S>(args...);
		state_ptr->fsm = this;
		_state_map[S::stateIndex()] = state_ptr; 
	}

	template<typename S>
    void transit(void) {
		assert(_state_map.count(S::stateIndex()));
		setCurrentState<S>();
	}

	template <typename S>
	void setCurrentState() {
		current_state_idx = S::stateIndex();
	}

	state_ptr_t getCurrentState() {
		assert(_state_map.count(current_state_idx));
		return _state_map[current_state_idx];
	}
	
protected:

	std::map<uint32_t, state_ptr_t> _state_map;
	std::unordered_map<CallbackKey,
					   std::vector<std::shared_ptr<EventCallbackBase> > > _callback_pool;
	uint32_t current_state_idx = -1;
};

template <typename S>
void BaseState::transit() {
	fsm->transit<S>();
}

template<typename T>
class State : public BaseState {
public:
	static Index stateIndex() {
		static Index state_index = state_index_counter++;
		return state_index;
	}	
};

}

#endif

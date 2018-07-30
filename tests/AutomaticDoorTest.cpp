#include <FSM.hpp>
#include <iostream>

// Define events
class OpenDoorEvent : public fsm::Event<OpenDoorEvent> {};
class CloseDoorEvent : public fsm::Event<CloseDoorEvent> {};
class LockDoorEvent : public fsm::Event<LockDoorEvent> {};
class UnlockDoorEvent : public fsm::Event<UnlockDoorEvent> {};

class DoorClosedState;
class DoorOpenState;
class DoorLockedState;

// Define states
class DoorClosedState : public fsm::State<DoorClosedState> {
public:
	void react(const OpenDoorEvent&) {
		transit<DoorOpenState>();
		std::cout << "Door is open" << std::endl;
	};

	void react(const LockDoorEvent&) {
		transit<DoorLockedState>();
		std::cout << "Door is locked" << std::endl;
	};
};

class DoorLockedState : public fsm::State<DoorLockedState> {
public:
	void react(const OpenDoorEvent&) {
		std::cout << "Can't open. Door is locked" << std::endl;
	};

	void react(const UnlockDoorEvent&) {
		transit<DoorClosedState>();		
		std::cout << "Door is unlocked" << std::endl;
	};

};

class DoorOpenState : public fsm::State<DoorOpenState> {
public:
	void react(const CloseDoorEvent&) {
		transit<DoorClosedState>();
		std::cout << "Door is closed" << std::endl;
	};
};

class Door {
public:

	Door() {
		// define graph states
		_fsm.addState<DoorClosedState>();
		_fsm.addState<DoorOpenState>();
		_fsm.addState<DoorLockedState>();

		// initial state
		_fsm.setCurrentState<DoorClosedState>();

		// subscribe events
		_fsm.subscribe<DoorClosedState, OpenDoorEvent>();
		_fsm.subscribe<DoorOpenState, CloseDoorEvent>();
		_fsm.subscribe<DoorClosedState, LockDoorEvent>();
		_fsm.subscribe<DoorLockedState, UnlockDoorEvent>();
		_fsm.subscribe<DoorLockedState, OpenDoorEvent>();
	}

	void open() {
		_fsm.dispatch(OpenDoorEvent());
	}

	void close() {
		_fsm.dispatch(CloseDoorEvent());
	}

	void lock() {
		_fsm.dispatch(LockDoorEvent());
	}

	void unlock() {
		_fsm.dispatch(UnlockDoorEvent());
	}
	

private:
	
	fsm::FSM _fsm;
};

int main() {	
	Door door;
	door.open();
	door.close();
	door.lock();
	door.open();
	door.unlock();
	door.open();	
}

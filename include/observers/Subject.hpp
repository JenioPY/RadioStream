#ifndef SUBJECT_HPP
#define SUBJECT_HPP
#include <vector>
#include "Observer.hpp"

class Subject
{
    using Context = State::Context;
public:
	void attach(std::unique_ptr<Observer> observer);
	void notify(const nana::widget& caller, Context context, events::Event e);
private:
	std::vector<std::unique_ptr<Observer>> observers_;
};

#endif

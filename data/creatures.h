#ifndef DATA_CREATURES_H
#define DATA_CREATURES_H

#include "combat.h"

#define CRTR_CONSTRUCTOR (string nm, Team *tm, Policy *pol = NULL, Combat *com = NULL): Creature(nm, tm, pol, com)

class Human: public Creature {
	public:
		static Weapon HAND;

		Human CRTR_CONSTRUCTOR {
			now = cap = {.health = 15, .magic = 30};
			stat = {.attack = 0, .defense = 10, .power = 0, .speed = 10};
			natural_weapons.push_back(&HAND);
		}
};

#endif

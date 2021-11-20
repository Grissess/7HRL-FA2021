#ifndef DATA_ITEMS_H
#define DATA_ITEMS_H

#include "combat.h"

class ChestPlate: public Item {
	public:
		string name() { return "chest plate"; }
		static Stats STATS;
		Stats *statBonus() { return &STATS; }

		static ChestPlate INSTANCE;
};

class Sword: public Item {
	public:
		string name() { return "sword"; }
		static Weapon WEAPON;
		Weapon *asWeapon() { return &WEAPON; }

		static Sword INSTANCE;
};

class WandOfDeletus: public Item {
	public:
		string name() { return "wand of deletus"; }
		static Weapon WEAPON;
		Weapon *asWeapon() { return &WEAPON; }
		static Stats STATS;
		Stats *statBonus() { return &STATS; }

		static WandOfDeletus INSTANCE;
};

#endif

#include <iostream>
#include <algorithm>

#include "sched.h"
#include "combat.h"
#include "entropy.h"

#include "data/creatures.h"
#include "data/items.h"

using namespace std;

class TestEvent: public PeriodicEvent {
	protected:
		unsigned left;
		double per, del;
		int pri;

	public:
		TestEvent(unsigned times, double p = 1.0, double d = 0.0, int pr = 0): left(times), per(p), del(d), pri(pr) {}

		double period() { return per; }
		double delay() { return del; }
		int priority() { return pri; }

		bool perform(double time) {
			PeriodicEvent::perform(time);
			cout << ((void *) this) << " performed at " << time
				<< " remaining " << --left << endl;
			return left == 0;
		}
};

int _test_scheduler() {
	Sched sched;

	TestEvent
		*e1 = new TestEvent(3),
		*e2 = new TestEvent(3, 1.0, 0.0, 1),
		*e3 = new TestEvent(3, 1.0, 0.0, -1),
		*e4 = new TestEvent(10, 0.3);

	sched.add(e1);
	sched.add(e2);
	sched.add(e3);
	sched.add(e4);

	sched.run();

	cout << "Done." << endl;

	return 0;
}

Uniform<int> sward_hit(1, 20), sward_amt(1, 8);
Damage sward_dmg = {
	.kind = DamageKind::SHARP,
	.amount = &sward_amt,
};
Weapon sward = {
	.name = string("sward"),
	.hit = &sward_hit,
	.damages = &sward_dmg,
	.n_damages = 1,
};
Consumed at = {.health = 100, .magic = 0}, bt = {.health = 75, .magic = 50};
Stats as = {
	.attack = 3, .defense = 12, .power = 5, .speed = 3
};
Stats bs = {
	.attack = 5, .defense = 10, .power = 7, .speed = 5
};

class ManualPolicy: public Policy {
	public:
		Action *select(Creature &creature) {
			cout << "It is now your turn: ";
			creature.describe(cout);
			cout << "." << endl;
			auto others = creature.combat->notInTeam(&creature);
			auto weapons = creature.weapons();
			if(others.empty() || weapons.empty()) {
				cout << "You can't take any other actions this turn; waiting by default." << endl;
				return new NoAction();
			}
			string action;
			while(true) {
				cout << "Choose an action: (w)ait, (a)ttack, (e)quip: ";
				cin >> action;
				switch(action[0]) {
					case 'w':
						cout << "You wait." << endl;
						return new NoAction;
					
					case 'a':
						while(true) {
							cout << "Your weapons:" << endl;
							size_t i = 0, weapon;
							for(auto weap: weapons) {
								cout << i++ << ": ";
								weap->describe(cout);
								cout << endl;
							}
							cout << "Using which weapon? ";
							cin >> i;
							if(i >= weapons.size()) {
								cout << "Invalid index." << endl;
								continue;
							}
							weapon = i;
							cout << "Targets:" << endl;
							i = 0;
							for(auto target: others) {
								cout << i++ << ": ";
								target->describe(cout);
								cout << endl;
							}
							cout << "Which target? ";
							cin >> i;
							if(i >= others.size()) {
								cout << "Invalid index." << endl;
								continue;
							}
							return new AttackAction(*others[i], *weapons[weapon]);
						}

					case 'e':
						while(true) {
							cout << "Your items:" << endl;
							size_t i = 0;
							for(auto item: creature.items) {
								cout << i++ << ": ";
								item->describe(cout);
								cout << endl;
							}
							cout << "Equip which item? ";
							cin >> i;
							if(i >= creature.items.size()) {
								cout << "Invalid index." << endl;
								continue;
							}
							return new EquipAction(creature.items[i]);
						}

					default:
						cout << "Try again." << endl;
				}
			}
		}
};

int _test_combat() {
	StreamLog slog(cout);
	Combat combat(&slog);
	Policy *pol = new BasicPolicy(), *man = new ManualPolicy();
	Team ta("team a"), tb("team b"), p("player");

	Creature *ca = new Human("A", &ta, pol);
	Creature *cb = new Human("B", &ta, pol);
	Creature *cp = new Human("Player", &p, man);

	ca->give(&Sword::INSTANCE);
	cb->give(&ChestPlate::INSTANCE);
	cb->equip(&ChestPlate::INSTANCE);
	cp->give(&Sword::INSTANCE);
	cp->give(&ChestPlate::INSTANCE);
	cp->give(&WandOfDeletus::INSTANCE);

	combat.add(ca);
	combat.add(cb);
	combat.add(cp);

	cout << "Starting with: ";
	for(auto c: combat.creatures) {
		c->describe(cout);
		cout << " ";
	}
	cout << endl;

	while(!combat.over()) {
		combat.doRound();
		cout << "At end of round:" << endl;
		for(auto c: combat.creatures) {
			c->describe(cout);
			cout << endl;
		}
	}

	cout << "Ending with: ";
	for(auto c: combat.creatures) {
		c->describe(cout);
		cout << " ";
	}
	cout << endl;

	cout << "Done." << endl;

	delete ca;
	delete cb;
	delete pol;
	delete man;

	return 0;
}

int main() {
	return _test_combat();
}

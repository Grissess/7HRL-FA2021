#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "combat.h"
#include "ansi.h"

using namespace std;

const string damageName(DamageKind kind) {
	switch(kind) {
		case DamageKind::BLUNT:
			return string("blunt");
		case DamageKind::SHARP:
			return string("sharp");
		case DamageKind::HEAT:
			return string("hot");
		case DamageKind::COLD:
			return string("cold");
		default:
			return string("unknown");
	}
}

double total_damage(Weapon *weapon) {
	double sum = 0.0;
	for(size_t i = 0; i < weapon->n_damages; i++) {
		sum += weapon->damages[i].amount->expected();
	}
	return sum;
}

int get_health(Creature *c) { return c->now.health; }

KeyComparator<Weapon *, double(*)(Weapon *)> BasicPolicy::best_weapon(total_damage);
KeyComparator<Creature *, int(*)(Creature *)> BasicPolicy::most_health(get_health);

void Consumed::describe(ostream &os) {
	os << "health=" << health << ",magic=" << magic;
}

void Stats::describe(ostream &os) {
	os << "attack=" << attack << ",defense=" << defense
		<< ",power=" << power << ",speed=" << speed;
}

void Item::describe(ostream &os) {
	os << C_ITEM << name() << C_RESET;
}

void Creature::describe(ostream &os) {
	Stats effStats = effectiveStats();

	os << C_CREATURE << "Creature(";
	if(isDead()) {
		os << "!DEAD! ";
	}
	os << name << "(" << team->name << ") ";
#ifdef FULL_INFO
	os << "now (";
	now.describe(os);
	os << ") cap (";
	cap.describe(os);
	os << ") base (";
	stat.describe(os);
	os << ") eff (";
	effStats.describe(os);
	os << ") ";
#else
	os << "H=" << now.health << "/" << cap.health << ","
		<< "M=" << now.magic << "/" << cap.magic << " "
#define stat(lbl, prop, term) \
		<< lbl "=" << effStats.prop << "(" << stat.prop << ")" term
		stat("ATK", attack, ",")
		stat("DEF", defense, ",")
		stat("PWR", power, ",")
		stat("SPD", speed, " ");
#undef stat
#endif
	os << "items {";
	string sep = "";
	for(auto item: items) {
		os << sep;
		item->describe(os);
		os << C_CREATURE;
		sep = ", ";
	}
	os << "} equipped {";
	sep = "";
	for(auto eq: equipped) {
		os << sep;
		eq->describe(os);
		os << C_CREATURE;
		sep = ", ";
	}
	os << "})" << C_RESET;
}

vector<Weapon *> Creature::weapons() {
	vector<Weapon *> v;
	for(auto weap: natural_weapons) {
		v.push_back(weap);
	}
	for(auto it: items) {
		Weapon *weap = it->asWeapon();
		if(weap) v.push_back(weap);
	}
	return v;
}

void Creature::take(Item *i) {
	auto iter = items.begin();
	while(iter != items.end()) {
		if(*iter == i) {
			items.erase(iter);
		} else {
			iter++;
		}
	}
	unequip(i);
}

bool Creature::equip(Item *i) {
	if(!i || !i->statBonus()) return false;
	if(find(items.begin(), items.end(), i) == items.end()) {
		return false;
	}
	equipped.push_back(i);
	return true;
}

void Creature::unequip(Item *i) {
	auto iter = equipped.begin();
	while(iter != equipped.end()) {
		if(*iter == i) {
			equipped.erase(iter);
		} else {
			iter++;
		}
	}
}

Stats Creature::effectiveStats() {
	Stats base = stat;
	for(auto item: equipped) {
		Stats *s = item->statBonus();
		if(s) {
			base += *s;
		}
	}
	return base;
}

Action *BasicPolicy::select(Creature &creature) {
	auto others = creature.combat->notInTeam(&creature);
	auto weapons = creature.weapons();
	if(others.empty() || weapons.empty()) {
		return new NoAction();
	} else {
		Weapon *best = *max_element(
				weapons.begin(), weapons.end(), best_weapon
		);
		Creature *target = *max_element(
				others.begin(), others.end(), most_health
		);
		return new AttackAction(*target, *best);
	}
}

void Weapon::describe(ostream &os) {
	os << C_ITEM << name << C_RESET;
}

void NoAction::act(Creature &performer, double time) {
	WaitCEvent ev(&performer);
	performer.combat->post(&ev);
}

bool ActionEvent::perform(double time) {
	if(perf.isDead()) {
		DiedCEvent ev(&perf);
		perf.combat->post(&ev);
	} else {
		act->act(perf, time);
	}
	return true;
}

void AttackAction::act(Creature &performer, double time) {
	Stats perf_stats = performer.effectiveStats();
	Stats target_stats = target.effectiveStats();
	int tohit = perf_stats.attack + weapon.hit->get();
	if(tohit >= target_stats.defense + target.attackResistance(weapon)) {
		for(size_t i = 0; i < weapon.n_damages; i++) {
			Damage *dm = weapon.damages + i;
			int amount = dm->amount->get() - target.attackReduction(dm->kind);
			if(amount < 0) amount = 0;
			target.damage(amount);
			HitCEvent ev(&performer, &target, amount, dm->kind, &weapon);
			performer.combat->post(&ev);
		}
	} else {
		MissCEvent ev(&performer, &target, &weapon);
		performer.combat->post(&ev);
	}
}

void EquipAction::act(Creature &performer, double time) {
	EquipCEvent ev(&performer, item, true);
	ev.success = performer.equip(item);
	performer.combat->post(&ev);
}

void WaitCEvent::describe(ostream &os) {
	os << C_CREATURE << nameOf(creature) << C_EVENT_NEUTRAL
		<< " waited" << C_RESET;
}

void MissCEvent::describe(ostream &os) {
	os << C_CREATURE << nameOf(attacker) << C_EVENT_MISS
		<< " missed " << C_CREATURE << nameOf(target);
	if(weapon) {
		os << " with ";
		weapon->describe(os);
	}
	os << C_RESET;
}

void HitCEvent::describe(ostream &os) {
	os << C_CREATURE << nameOf(attacker) << C_EVENT_DAMAGE
		<< " hit " << C_CREATURE << nameOf(target)
		<< C_EVENT_DAMAGE << " for "
		<< amount << " " << damageName(kind) << " damage";
	if(weapon) {
		os << " with ";
		weapon->describe(os);
	}
	os << C_RESET;
}

void EquipCEvent::describe(ostream &os) {
	os << C_CREATURE << nameOf(equipper);
	if(success) {
		os << C_EVENT_NEUTRAL << " equipped ";
	} else {
		os << C_EVENT_MISS << " tried to equip ";
	}
	if(item) {
		item->describe(os);
		os << (success ? C_EVENT_NEUTRAL : C_EVENT_MISS);
	} else {
		os << "[a NULL item, uh oh!]";
	}
	if(!success) {
		os << ", but it failed";
	}
	os << C_RESET;
}

void DiedCEvent::describe(ostream &os) {
	os << C_CREATURE << nameOf(dead) << C_EVENT_DAMAGE
		<< " died before their turn";
}

void StreamLog::accept(CombatEvent *ev) {
	ev->describe(os);
	os << "!" << endl;
}

vector<Creature *> Combat::living() {
	vector<Creature *> v;
	for(auto c: creatures) {
		if(!c->isDead()) {
			v.push_back(c);
		}
	}
	return v;
}

vector<Creature *> Combat::notInTeam(Creature *creature) {
	vector<Creature *> v;
	for(auto c: living()) {
		if(c->team != creature->team) {
			v.push_back(c);
		}
	}
	return v;
}

void Combat::doRound() {
	vector<Event *> events;
	vector<Action *> actions;

	for(auto c: creatures) {
		if(!c->policy || c->isDead()) continue;
		Action *act = c->policy->select(*c);
		actions.push_back(act);
		Event *ev = new ActionEvent(*c, act, -c->effectiveStats().speed);
		events.push_back(ev);
		sched.add(ev);
	}

	sched.run();

	// Now that the scheduler is empty...
	for(auto ev: events) {
		delete ev;
	}
	for(auto act: actions) {
		delete act;
	}

	round++;
}

bool Combat::over() {
	vector<Creature *> alive = living();
	if(alive.empty()) return true;
	if(notInTeam(alive.front()).empty()) return true;
	return false;
}

void Combat::run() {
	while(!over()) doRound();
}

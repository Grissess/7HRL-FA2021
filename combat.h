#ifndef COMBAT_H
#define COMBAT_H

#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

#include "sched.h"
#include "entropy.h"

using namespace std;

enum class DamageKind {
	BLUNT, SHARP,
	HEAT, COLD,
};

const string damageName(DamageKind);

class Combat;

struct Consumed {
	int health, magic;
	void describe(ostream &);
};

struct Stats {
	int attack, defense, power, speed;
	void describe(ostream &);
	Stats operator+(const Stats &rhs) { return {
		.attack = attack + rhs.attack,
		.defense = defense + rhs.defense,
		.power = power + rhs.power,
		.speed = speed + rhs.speed,
	};}

	Stats &operator+=(const Stats &rhs) {
		attack += rhs.attack;
		defense += rhs.defense;
		power += rhs.power;
		speed += rhs.speed;
		return *this;
	}
};

class Action;
class Creature;
struct Weapon;

class Policy {
	public:
		virtual Action *select(Creature &creature) = 0;
};

class Team {
	public:
		string name;

		Team(string nm): name(nm) {}
};

class Item {
	public:
		virtual string name() = 0;
		virtual bool use(Creature &user) { return false; };
		virtual Weapon *asWeapon() { return NULL; };
		virtual Stats *statBonus() { return NULL; };
		virtual void describe(ostream &);
};

class Creature {
	public:
		Consumed now, cap;
		Stats stat;
		Combat *combat;
		Policy *policy;
		string name;
		Team *team;
		vector<Item *> items;
		vector<Weapon *> natural_weapons;
		vector<Item *> equipped;

		Creature(string nm, Team *tm, Policy *pol, Combat *com = NULL): combat(com), policy(pol), team(tm), name(nm) {}

		virtual void damage(int amount) { if(amount > 0) now.health = max(now.health - amount, 0); }
		virtual void heal(int amount) { if(amount > 0) now.health = min(now.health + amount, cap.health); }
		virtual bool isDead() { return now.health <= 0; }

		virtual int attackResistance(Weapon &weapon) { return 0; }
		virtual int attackReduction(DamageKind kind) { return 0; }

		virtual void give(Item *i) { items.push_back(i); }
		virtual void take(Item *i);
		virtual bool equip(Item *i);
		virtual void unequip(Item *i);
		virtual vector<Weapon *> weapons();

		virtual Stats effectiveStats();

		virtual void describe(ostream &);
};

double total_damage(Weapon *);
int get_health(Creature *);

class BasicPolicy: public Policy {
	public:
		static KeyComparator<Weapon *, double(*)(Weapon *)> best_weapon;
		static KeyComparator<Creature *, int(*)(Creature *)> most_health;

		Action *select(Creature &creature);
};

class Action {
	public:
		virtual void act(Creature &performer, double time) = 0;
};

class WaitCEvent;

class NoAction: public Action {
	public:
		void act(Creature &performer, double time);
};

class ActionEvent: public Event {
	protected:
		double next;
		int pri;
		Action *act;
		Creature &perf;

	public:
		ActionEvent(Creature &c, Action *a, int p): act(a), pri(p), perf(c) {}

		void initTimebase(double origin) { next = origin; }
		double nextTime() { return next; }
		int priority() { return pri; }
		bool perform(double time);
};


struct Damage {
	DamageKind kind;
	Roll<int> *amount;
};

struct Weapon {
	string name;
	Roll<int> *hit;
	Damage *damages;
	size_t n_damages;
	void describe(ostream &);
};

class AttackAction: public Action {
	public:
		Creature &target;
		Weapon &weapon;

		AttackAction(Creature &tar, Weapon &weap): target(tar), weapon(weap) {}

		void act(Creature &performer, double time);
};

class EquipAction: public Action {
	public:
		Item *item;

		EquipAction(Item *i): item(i) {}

		void act(Creature &performer, double time);
};

class CombatEvent {
	public:
		virtual void describe(ostream &os) = 0;

		static string nameOf(Creature *c) {
			return c ? c->name : string("unknown creature");
		}
};

class WaitCEvent: public CombatEvent {
	public:
		Creature *creature;

		WaitCEvent(Creature *c = NULL): creature(c) {}

		void describe(ostream &os);
};

class MissCEvent: public CombatEvent {
	public:
		Creature *attacker, *target;
		Weapon *weapon;

		MissCEvent(Creature *a = NULL, Creature *t = NULL, Weapon *w = NULL): attacker(a), target(t), weapon(w) {}

		void describe(ostream &os);
};

class HitCEvent: public CombatEvent {
	public:
		Creature *attacker, *target;
		int amount;
		DamageKind kind;
		Weapon *weapon;

		HitCEvent(Creature *a = NULL, Creature *t = NULL, int am = 0, DamageKind k = DamageKind::BLUNT, Weapon *w = NULL): attacker(a), target(t), amount(am), kind(k), weapon(w) {}

		void describe(ostream &os);
};

class EquipCEvent: public CombatEvent {
	public:
		Creature *equipper;
		Item *item;
		bool success;

		EquipCEvent(Creature *e, Item *i, bool s): equipper(e), item(i), success(s) {}

		void describe(ostream &os);
};

class DiedCEvent: public CombatEvent {
	public:
		Creature *dead;

		DiedCEvent(Creature *d = NULL): dead(d) {}

		void describe(ostream &os);
};

class Log {
	public:
		virtual void accept(CombatEvent *ev) = 0;
};

class StreamLog: public Log {
	public:
		ostream &os;

		StreamLog(ostream &o): os(o) {}

		void accept(CombatEvent *ev);
};

class Combat {
	public:
		vector<Creature *> creatures;
		Log *log;
		Sched sched;
		int round;

		Combat(Log *l): log(l), round(0) {}

		void add(Creature *c) {
			c->combat = this;
			creatures.push_back(c);
		}
		void post(CombatEvent *ev) { log->accept(ev); }
		vector<Creature *> living();
		vector<Creature *> notInTeam(Creature *creature);
		void doRound();
		bool over();
		void run();
};

#endif

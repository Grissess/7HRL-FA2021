#include <vector>
#include <iostream>
#include <unordered_map>

#include "map.h"

using namespace std;

void World::fill(World::R rect, Tile with) {
	using V = World::V;
	using I = World::index;

	V min = rect.origin(), max = rect.opposite();
	for(I y = min.y; y < max.y; y++) {
		for(I x = min.x; x < max.x; x++) {
			grid[x, y] = with;
		}
	}
}

World::posmap World::actorPosMap() {
	World::posmap map;

	for(auto actor: actors) {
		map[actor->pos.x][actor->pos.y] = actor;
	}

	return map;
}

void World::draw(ostream &os) {
	using I = World::index;
	using V = World::V;

	World::posmap pmap = actorPosMap();

	os << A_CLEAR A_HOME;
	V sz = screen.size(), scrc = screen.size() / V(2);
	for(I y = 0; y < max.y; y++) {
		for(I x = 0; x < max.x; x++) {
			V wpos = V2(x, y) - scrc + center;
			Actor *a = NULL;

			auto slice = pmap.find(wpos.x);
			if(slice != pmap.end()) {
				auto cell = slice->find(wpos.y);
				if(cell != slice->end()) {
					a = *cell;
				}
			}

			if(a) {
				cout << a->render();
			} else {
				Tile *t = grid[wpos];
				if(t) {
					cout << t->render();
				} else {
					cout << ' ';
				}
			}
		}
	}
}

void World::add(Actor *a) {
	actors.push_back(a);
}

void World::tick() {
}

void World::schedule(ActorAction *act) {
	asdf

Grid::V motionOffset(Motion motion) {
	using V = Grid::V;

	switch(motion) {
		case UP:
			return V(0, -1);
		case UPRIGHT:
			return V(1, -1);
		case RIGHT:
			return V(1, 0);
		case DOWNRIGHT:
			return V(1, 1);
		case DOWN:
			return V(0, 1);
		case DOWNLEFT:
			return V(-1, 1);
		case LEFT:
			return V(-1, 0);
		case UPLEFT:
			return V(-1, -1);
		default:
			return V();
	}
}

CreatureActor::CreatureActor(World *w, CreatureActor::V p, Creature c, ActorPolicy *pol, Renderer *r = NULL): Actor(w, p, r), creature(c), policy(pol) {
	world->actorOfCreature[&creature] = this;
	creature.policy = &world->policy;
}

void CreatureActor::tick() {
	if(!creature.combat && policy) {
		ActorAction *act = policy->select(this);
		if(act) {
			world

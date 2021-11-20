#include "data/creatures.h"

static Uniform<int> TYPICAL_HIT(1, 20);

Uniform<int> HAND_DIE(1, 4);
Damage HAND_DMG = {.kind = DamageKind::BLUNT, .amount = &HAND_DIE};
Weapon Human::HAND = { .name = string("hand"),
	.hit = &TYPICAL_HIT, .damages = &HAND_DMG, .n_damages = 1
};

#include "data/items.h"

static Uniform<int> TYPICAL_HIT(1, 20);

Stats ChestPlate::STATS = {
	.attack = 0, .defense = 5, .power = 0, .speed = -2
};

Uniform<int> SWORD_DIE(1, 8);
Damage SWORD_DMG = {.kind = DamageKind::SHARP, .amount = &SWORD_DIE};
Weapon Sword::WEAPON = { .name = string("sword"),
	.hit = &TYPICAL_HIT, .damages = &SWORD_DMG, .n_damages = 1
};

Uniform<int> WAND_DIE(1000, 1000);
Damage WAND_DMG[] = {
#define dmg(k) \
	{.kind = DamageKind::k, .amount = &WAND_DIE}
	dmg(BLUNT),
	dmg(SHARP),
	dmg(HEAT),
	dmg(COLD)
#undef dmg
};
Linear<int> WAND_HIT(&TYPICAL_HIT, 1000);
Weapon WandOfDeletus::WEAPON = { .name = string("wand of deletus"),
	.hit = &WAND_HIT, .damages = WAND_DMG,
	.n_damages = sizeof(WAND_DMG)/sizeof(*WAND_DMG)
};
Stats WandOfDeletus::STATS = {
	.attack = 1000, .defense = 1000, .power = 1000, .speed = 1000
};

ChestPlate ChestPlate::INSTANCE;
Sword Sword::INSTANCE;
WandOfDeletus WandOfDeletus::INSTANCE;

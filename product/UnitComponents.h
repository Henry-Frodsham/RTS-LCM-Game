#pragma once
#include <vector>

struct HealthComponent {
  float Health;
  HealthComponent(float HP) : Health(HP) {}
};

struct AttackComponent {
  float Radius;
  float Damage;
  AttackComponent(int R, int D) : Radius(R), Damage(D) {}
};

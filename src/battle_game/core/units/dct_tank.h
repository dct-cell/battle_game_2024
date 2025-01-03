#pragma once
#include "battle_game/core/unit.h"

namespace battle_game::unit {
class Dct_Tank : public Unit {
 public:
  Dct_Tank(GameCore *game_core, uint32_t id, uint32_t player_id);
  void Render() override;
  void Update() override;
  [[nodiscard]] bool IsHit(glm::vec2 position) const override;

 protected:
  void TankMove(float move_speed, float rotate_angular_speed);
  void TurretRotate();
  void Fire();
  [[nodiscard]] const char *UnitName() const override;
  [[nodiscard]] const char *Author() const override;

  float turret_rotation_{0.0f};
  uint32_t fire_count_down_{0};
  uint32_t skill_count_down_{0};
  uint32_t mine_count_down_{0};
  uint32_t back_count_down_{0};
  uint32_t trans_count_down_{0};
  uint32_t trans_step_{0};
  uint32_t meteo_count_down_{0};
  uint32_t change_count_down_{0};
  bool fire_pattern{0};
};
}  // namespace battle_game::unit

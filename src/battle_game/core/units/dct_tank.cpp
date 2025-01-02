#include "tiny_tank.h"

#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t tank_body_model_index = 0xffffffffu;
uint32_t tank_turret_model_index = 0xffffffffu;
}  // namespace

Dct_Tank::Dct_Tank(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~tank_body_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      /* Tank Body */
      tank_body_model_index = mgr->RegisterModel(
          {
              {{-1.1f, 0.6f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{-1.1f, -0.6f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{-0.7f, -0.9f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{0.7f, -0.9f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{1.1f, -0.6f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{1.1f, 0.6f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{0.7f, 0.9f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{0.5f, 0.6f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{-0.5f, 0.6f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
              {{-0.7f, 0.9f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
          },
          {0, 8, 9, 7, 6, 5, 0, 1, 5, 1, 4, 5, 1, 4, 2, 2, 3, 4});
    }

    {
      /* Tank Turret */
      std::vector<ObjectVertex> turret_vertices;
      std::vector<uint32_t> turret_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        turret_vertices.push_back({{sin_theta * 0.5f, cos_theta * 0.5f},
                                   {0.0f, 0.0f},
                                   {0.7f, 0.7f, 0.7f, 1.0f}});
        turret_indices.push_back(i);
        turret_indices.push_back((i + 1) % precision);
        turret_indices.push_back(precision);
      }
      turret_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_indices.push_back(precision + 1 + 0);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 3);
      tank_turret_model_index =
          mgr->RegisterModel(turret_vertices, turret_indices);
    }
  }
}

void Dct_Tank::Render() {
  battle_game::SetTransformation(position_, rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(tank_body_model_index);
  battle_game::SetRotation(turret_rotation_);
  battle_game::DrawModel(tank_turret_model_index);
}

void Dct_Tank::Update() {
  TankMove(3.0f, glm::radians(180.0f));
  TurretRotate();
  Fire();
}

void Dct_Tank::TankMove(float move_speed, float rotate_angular_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    glm::vec2 offset{0.0f};
    if (input_data.key_down[GLFW_KEY_W] && input_data.key_down[GLFW_KEY_X]){
      offset.y += 3.0f;
    }else if (input_data.key_down[GLFW_KEY_W]) {
      offset.y += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_S] && input_data.key_down[GLFW_KEY_X]){
      offset.y -= 3.0f;
    }else if(input_data.key_down[GLFW_KEY_S]){
      offset.y -= 1.0f;
    }
    if(trans_step_ % 5 == 1){
      offset.y += 4.0f;
    }
    float speed = move_speed * GetSpeedScale();
    offset *= kSecondPerTick * speed;
    auto new_position =
        position_ + glm::vec2{glm::rotate(glm::mat4{1.0f}, rotation_,
                                          glm::vec3{0.0f, 0.0f, 1.0f}) *
                              glm::vec4{offset, 0.0f, 0.0f}};
    if (!game_core_->IsBlockedByObstacles(new_position)) {
      game_core_->PushEventMoveUnit(id_, new_position);
    }
    float rotation_offset = 0.0f;
    if (input_data.key_down[GLFW_KEY_A]) {
      rotation_offset += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_D]) {
      rotation_offset -= 1.0f;
    } 
    rotation_offset *= kSecondPerTick * rotate_angular_speed * GetSpeedScale();
    if (input_data.key_down[GLFW_KEY_B]) {
      if (back_count_down_ == 0){
        rotation_offset += glm::pi<float>() * 1.0f;
        back_count_down_ = kTickPerSecond / 2;  // Fire interval 0.5 second.
      } 
    }
    if (input_data.key_down[GLFW_KEY_T]) {
      if (trans_count_down_ == 0){
        trans_count_down_ = kTickPerSecond * 10;  // Fire interval 10 second.
        trans_step_ = kTickPerSecond * 1;
      } 
    }
    if(trans_count_down_) {
      trans_count_down_--;
    }
    if (back_count_down_) {
      back_count_down_--;
    }
    if (trans_step_) {
      trans_step_--;
    }
    game_core_->PushEventRotateUnit(id_, rotation_ + rotation_offset);
  }
}

void Dct_Tank::TurretRotate() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    auto diff = input_data.mouse_cursor_position - position_;
    if (glm::length(diff) < 1e-4) {
      turret_rotation_ = rotation_;
    } else {
      turret_rotation_ = std::atan2(diff.y, diff.x) - glm::radians(90.0f);
    }
  }
}

void Dct_Tank::Fire() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    if(input_data.key_down[GLFW_KEY_Q]){
      fire_pattern = !fire_pattern;
    }
    if (fire_count_down_ == 0){
      if (input_data.mouse_button_down[GLFW_MOUSE_BUTTON_LEFT]){
        auto velocity = Rotate(glm::vec2{0.0f, 13.0f}, turret_rotation_);
        if(fire_pattern){
          GenerateBullet<bullet::EvoCannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = 2 * kTickPerSecond;  // Fire interval 2 second.
        }else{
          GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
          fire_count_down_ = kTickPerSecond;  // Fire interval 1 second.
        }
      }  
    }
    if(input_data.key_down[GLFW_KEY_E]){
      if (skill_count_down_ == 0){
        auto velocity = Rotate(glm::vec2{0.0f, 10.0f}, turret_rotation_);
        auto alpha = glm::pi<float>() * 0.1f;
        for(int k = 0 ; k < 20 ; k++){
          GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_ + k * alpha),
            turret_rotation_ + k * alpha, GetDamageScale(), Rotate(velocity, turret_rotation_ + k * alpha));
        }
        fire_count_down_ = kTickPerSecond;  // Fire interval 1 second.
        skill_count_down_ = 10 * kTickPerSecond;  // Skill interval 10 second.
      }
    }
    if(input_data.key_down[GLFW_KEY_K]){
      if(meteo_count_down_ == 0){
        auto velocity = Rotate(glm::vec2{0.0f, 5.0f}, turret_rotation_);
        GenerateBullet<bullet::Meteorite>(
          position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
          turret_rotation_, GetDamageScale(), velocity);
        meteo_count_down_ = 20 * kTickPerSecond;  // Fire interval 20 second.
      } 
    }
    if(trans_step_ % 5 == 1){
      auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, rotation_);
      auto beta = glm::pi<float>() * 0.5f;
      GenerateBullet<bullet::CannonBall>(
          position_ + Rotate({0.0f, 1.2f}, rotation_ + beta),
          rotation_ + beta, GetDamageScale(), Rotate(velocity,  beta));
      GenerateBullet<bullet::CannonBall>(
          position_ + Rotate({0.0f, 1.2f}, rotation_ - beta),
          rotation_ - beta, GetDamageScale(), Rotate(velocity, -beta));
    }
  }
  if (fire_count_down_) {
    fire_count_down_--;
  }
  if (skill_count_down_) {
    skill_count_down_--;
  }
  if (meteo_count_down_) {
    meteo_count_down_--;
  }
}

bool Dct_Tank::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.x > -0.8f && position.x < 0.8f && position.y > -1.0f &&
         position.y < 1.0f && position.x + position.y < 1.6f &&
         position.y - position.x < 1.6f;
}

const char *Dct_Tank::UnitName() const {
  return "Dct_Tank";
}

const char *Dct_Tank::Author() const {
  return "Yanmohan Wang";
}
}  // namespace battle_game::unit

#include <SokuLib.hpp>

/*#######################################################*/

THMemPatcher2 mod;

hook_t<vtable::InfoManager, offset::InfoManager::RenderHud> draw_hud_hook;
hook_t<vtable::InfoManagerStory, offset::InfoManager::RenderHud> draw_story_hud_hook;

patch_nop_t<0x0047A9BF, 5> disable_characters_and_bg_draw;

patch_t<0x0047DB95, 
	0x5F,			/*pop edi*/
	0x5E,			/*pop esi*/
	0x5B,			/*pop ebx*/
	0x83, 0xC4, 0x14,	/*add esp, 0x14*/
	0xC3,			/*retn*/
	0x90, 0x90, 0x90>	/*nop*/
	draw_hud_custom_prolog;

auto& renderer = Soku::GetRenderer();
auto& bg_renderer = Soku::GetBgRenderer();
auto& hud_renderer = Soku::GetHudRenderer();
auto& common_fx_renderer = Soku::GetFxRenderer();
auto& character_renderer = Soku::GetCharacterRenderer();

/*#######################################################*/

constexpr auto CalculateElapsedFactor = mem_cast<double(__cdecl*)(int)>(0x004096D0u);
constexpr auto hud_hide_offset = mem_cast<double*>(0x00857948u);

inline float get_hud_y_offset() {
	return  (float)(*hud_hide_offset * (1.f - CalculateElapsedFactor(3 * hud_renderer->animation_time)));
}

/*#######################################################*/

/* EXAMPLE */
void vanilla_bg_and_characters_draw() {
	bg_renderer->draw_stage_bg();
	renderer.set_blend_mode(BlendingMode::normal);
	renderer.set_blend_ex(2);
	character_renderer->draw_shadow();
	renderer.set_blend_ex(1);
	bg_renderer->draw_animated_elements();
	character_renderer->draw_misc(CharacterFxType::spellcard_bg);
	character_renderer->draw_spellcard_portrait();
	character_renderer->draw_spellcard_bg();
	common_fx_renderer->draw(CommonFxType::jump);
	character_renderer->draw_misc(CharacterFxType::bullets_bg);
	character_renderer->draw_character();
	bg_renderer->draw_weather_fx();
	character_renderer->draw_misc(CharacterFxType::bullets);
	character_renderer->draw_misc(CharacterFxType::spell_break);
	common_fx_renderer->draw(CommonFxType::weather_crystals_and_dash);
	character_renderer->draw_misc(CharacterFxType::sanae_hud);
}

/* EXAMPLE */
void vanilla_hud_draw() {
	float offset = get_hud_y_offset();
	//renderer.set_render_mode(RenderMode::full);
	//renderer.set_blend_mode(BlendingMode::normal);
	//hud_renderer->portrait_p1->draw(0.f, -offset);
	//hud_renderer->portrait_p2->draw(640.f, -offset);
	//hud_renderer->upper_hud.draw_ex(0.f, -offset);
	//draw names
	hud_renderer->draw_hud(); //ADDR_DRAW_HUD: need finish reverse for names draw
	hud_renderer->under_hud.draw(0.f, offset);
	hud_renderer->fx.draw(HudFxType::orb_bg);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->hit_counter_p1.draw();
	hud_renderer->hit_counter_p2.draw();
	hud_renderer->cards_p1.draw(offset);
	hud_renderer->cards_p2.draw(offset);
	hud_renderer->card_name_p1.draw();
	hud_renderer->card_name_p2.draw();
	hud_renderer->fx.draw(HudFxType::cards_orb_etc);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->upper_hud.draw(0.f, -offset);
	hud_renderer->fx.draw(HudFxType::round_alert_bgm);
}

/* EXAMPLE */
void vanilla_hud_story_draw() {
	float offset = get_hud_y_offset();
	renderer.set_render_mode(RenderMode::full);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->portrait_p1->draw(0.f, -offset);
	hud_renderer->portrait_p2->draw(640.f, -offset);
	hud_renderer->upper_hud.draw(0.f, -offset);
	hud_renderer->under_hud.draw(0.f, offset);
	character_renderer->draw_misc(CharacterFxType::spell_break);
	hud_renderer->hit_counter_p1.draw();
	hud_renderer->hit_counter_p2.draw();
	hud_renderer->cards_p1.draw(offset);
	hud_renderer->card_name_p1.draw();
	hud_renderer->card_name_p2.draw();
	hud_renderer->fx.draw(HudFxType::cards_orb_etc);
	hud_renderer->fx.draw(HudFxType::round_alert_bgm);
}

/*#######################################################*/

int8_t __fastcall draw_orb_fx_callback(DWORD fx, int /*edx*/, int8_t type) {
	return !hud_renderer->is_card_sprite(fx) ? Soku::draw_hud_fx_callback(fx, type) : type;
}

int8_t __fastcall draw_card_fx_callback(DWORD fx, int /*edx*/, int8_t type) {
	return hud_renderer->is_card_sprite(fx) ? Soku::draw_hud_fx_callback(fx, type) : type;
}

void draw_bg() {
	bg_renderer->draw_stage_bg();
	renderer.set_blend_mode(BlendingMode::normal);
	renderer.set_blend_ex(2);
	character_renderer->draw_shadow();
	renderer.set_blend_ex(1);
	bg_renderer->draw_animated_elements();
	character_renderer->draw_misc(CharacterFxType::spellcard_bg);
	character_renderer->draw_spellcard_portrait();
	character_renderer->draw_spellcard_bg();
	bg_renderer->draw_weather_fx();
	character_renderer->draw_misc(CharacterFxType::spell_break);
}

void draw_characters() {
	common_fx_renderer->draw(CommonFxType::jump);
	character_renderer->draw_misc(CharacterFxType::bullets_bg);
	character_renderer->draw_character();
	character_renderer->draw_misc(CharacterFxType::bullets);
	common_fx_renderer->draw(CommonFxType::weather_crystals_and_dash);
	renderer.set_blend_mode(BlendingMode::normal);
	character_renderer->draw_misc(CharacterFxType::sanae_hud);
}

void draw_upper_hud() {
	hud_renderer->fx.draw(HudFxType::orb_bg);
	hud_renderer->fx_ex.draw((DrawCallbackFn)draw_orb_fx_callback, 0, HudFxType::cards_orb_etc);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->upper_hud.draw(0.f, -get_hud_y_offset()); //-0.0f is a necessary
	hud_renderer->hit_counter_p1.draw();
	hud_renderer->hit_counter_p2.draw();
	hud_renderer->card_name_p1.draw();
	hud_renderer->card_name_p2.draw();
}

void draw_under_hud() {
	float offset = get_hud_y_offset();
	hud_renderer->under_hud.draw(0.f, offset);
	hud_renderer->cards_p1.draw(offset);
	hud_renderer->cards_p2.draw(offset);
	hud_renderer->fx_ex.draw((DrawCallbackFn)draw_card_fx_callback, 0, HudFxType::cards_orb_etc);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->fx.draw(HudFxType::round_alert_bgm);
	renderer.set_blend_mode(BlendingMode::normal);
}

void draw_story_upper_hud() {
	float offset = get_hud_y_offset();
	renderer.set_render_mode(RenderMode::full);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->portrait_p1->draw(0.f, -offset);
	hud_renderer->portrait_p2->draw(640.f, -offset);
	hud_renderer->upper_hud.draw(0.f, -offset);
	character_renderer->draw_misc(CharacterFxType::spell_break);
	hud_renderer->hit_counter_p1.draw();
	hud_renderer->hit_counter_p2.draw();
	hud_renderer->card_name_p1.draw();
	hud_renderer->card_name_p2.draw();
	hud_renderer->fx_ex.draw((DrawCallbackFn)draw_orb_fx_callback, 0, HudFxType::cards_orb_etc);
	renderer.set_blend_mode(BlendingMode::normal);
}

void draw_story_under_hud() {
	float offset = get_hud_y_offset();
	hud_renderer->under_hud.draw(0.f, offset);
	hud_renderer->cards_p1.draw(offset);
	hud_renderer->fx_ex.draw((DrawCallbackFn)draw_card_fx_callback, 0, HudFxType::cards_orb_etc);
	renderer.set_blend_mode(BlendingMode::normal);
	hud_renderer->fx.draw(HudFxType::round_alert_bgm);
	renderer.set_blend_mode(BlendingMode::normal);
}

/*#######################################################*/

def_hook(draw_hud_hook)(void* _this) {
	draw_bg();
	draw_hud_hook(_this); //modifed ADDR_DRAW_HUD: need finish reverse for names draw
	draw_upper_hud();
	draw_characters();
	draw_under_hud();
}

def_hook(draw_story_hud_hook)(void*) {
	draw_bg();
	draw_story_upper_hud();
	draw_characters();
	draw_story_under_hud();
}

/*#######################################################*/

bool Initialize(HMODULE, HMODULE) {
	// patches for native functions
	mod.AddPatch(disable_characters_and_bg_draw);
	mod.AddPatch(draw_hud_custom_prolog);
	mod.AddHook(draw_hud_hook);
	mod.AddHook(draw_story_hud_hook);
	mod.ApplyPatches();
	return true;
}

void AtExit() {
	mod.ClearPatches();
}

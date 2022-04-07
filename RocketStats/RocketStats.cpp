/* ===========================================================
 *   Developped by @Lyliya, @NuSa_yt, @Arubinu42 & @Larsluph
 * =========================================================== */

#include "RocketStats.h"

BAKKESMOD_PLUGIN(RocketStats, "RocketStats", "4.0.6", PERMISSION_ALL)

#pragma region Utils
Stats RocketStats::GetStats()
{
    Stats result;

    switch (rs_mode)
    {
        case 1: result = stats[current.playlist]; break;
        case 2: result = always; break;
        case 3: result = always_gm[current.playlist]; break;
        default: result = session;
    }

    return result;
}

std::string RocketStats::GetRank(int tierID)
{
    cvarManager->log("Tier: " + std::to_string(tierID));
    if (tierID < rank_nb)
        return rank[tierID].name;
    else
        return "Unranked";
}

std::string RocketStats::GetPlaylistName(int playlistID)
{
    if (playlist_name.find(playlistID) != playlist_name.end())
        return playlist_name.at(playlistID);
    else
        return "Unknown Game Mode";
}

void RocketStats::LogImageLoadStatus(bool status, std::string imageName)
{
    if (status)
        cvarManager->log(imageName + ": image load");
    else
        cvarManager->log(imageName + ": failed to load");
}

std::shared_ptr<ImageWrapper> RocketStats::LoadImg(const std::string& _filename)
{
    fs::path _path = GetPath(_filename);
    return LoadImg(_path);
}

std::shared_ptr<ImageWrapper> RocketStats::LoadImg(fs::path& _path)
{
    return std::make_shared<ImageWrapper>(_path, false, true);
}

void RocketStats::LoadImgs()
{
    int load_check = 0;

    casual = LoadImg("RocketStats_images/Casual.png");
    load_check += (int)casual->IsLoadedForImGui();
    if (!load_check)
        cvarManager->log("Casual: failed to load");

    for (int i = 0; i < rank_nb; ++i)
    {
        rank[i].image = LoadImg("RocketStats_images/" + rank[i].name + ".png");
        load_check += (int)rank[i].image->IsLoadedForImGui();
        LogImageLoadStatus(rank[i].image->IsLoadedForImGui(), rank[i].name);
    }
    cvarManager->log(std::to_string(load_check) + "/" + std::to_string(rank_nb + 1) + " images were loaded successfully");
}

bool RocketStats::GetCVar(const char* name, int& value)
{
    std::string key = (name + 3);
    if (themes_values[theme_render.name].is_object() && (themes_values[theme_render.name][key].is_number_integer() || themes_values[theme_render.name][key].is_number_unsigned()))
    {
        value = int(themes_values[theme_render.name][key]);
        cvarManager->log("GetCVar: " + std::string(name) + " " + std::to_string(value));

        return true;
    }

    return false;
}

bool RocketStats::GetCVar(const char* name, bool& value)
{
    std::string key = (name + 3);
    if (themes_values[theme_render.name].is_object() && themes_values[theme_render.name][key].is_boolean())
    {
        value = bool(themes_values[theme_render.name][key]);
        cvarManager->log("GetCVar: " + std::string(name) + " " + std::to_string(value));

        return true;
    }

    return false;
}

bool RocketStats::GetCVar(const char* name, float& value)
{
    std::string key = (name + 3);
    if (themes_values[theme_render.name].is_object() && themes_values[theme_render.name][key].is_number())
    {
        value = float(themes_values[theme_render.name][key]);
        cvarManager->log("GetCVar: " + std::string(name) + " " + std::to_string(value));

        return true;
    }

    return false;
}

bool RocketStats::SetCVar(const char* name, int& value, bool save)
{
    std::string key = (name + 3);
    if (value != cvarManager->getCvar(name).getIntValue())
    {
        cvarManager->log("SetCVar: " + std::string(name) + " " + std::to_string(value));
        cvarManager->getCvar(name).setValue(value);

        if (save)
        {
            if (!themes_values[theme_render.name].is_object())
                themes_values[theme_render.name] = json::object();

            themes_values[theme_render.name][key] = value;
        }

        return true;
    }

    return false;
}

bool RocketStats::SetCVar(const char* name, bool& value, bool save)
{
    std::string key = (name + 3);
    if (value != cvarManager->getCvar(name).getBoolValue())
    {
        cvarManager->log("SetCVar: " + std::string(name) + " " + std::to_string(value));
        cvarManager->getCvar(name).setValue(value);

        if (save)
        {
            if (!themes_values[theme_render.name].is_object())
                themes_values[theme_render.name] = json::object();

            themes_values[theme_render.name][key] = value;
        }

        return true;
    }

    return false;
}

bool RocketStats::SetCVar(const char* name, float& value, bool save)
{
    std::string key = (name + 3);
    value = (std::round(value * 1000.f) / 1000.f);
    if (value != cvarManager->getCvar(name).getFloatValue())
    {
        cvarManager->log("SetCVar: " + std::string(name) + " " + std::to_string(value));
        cvarManager->getCvar(name).setValue(value);

        if (save)
        {
            if (!themes_values[theme_render.name].is_object())
                themes_values[theme_render.name] = json::object();

            themes_values[theme_render.name][key] = value;
        }

        return true;
    }

    return false;
}

void RocketStats::CloseWelcome()
{
    if (rs_recovery == 1)
    {
        rs_recovery = 2;
        RemoveFile("RocketStats_images/welcome.png");

        gameWrapper->SetTimeout([&](GameWrapper* gameWrapper) {
            cvarManager->executeCommand("exec config");

            gameWrapper->SetTimeout([&](GameWrapper* gameWrapper) {
                rs_launch = 0.f;
                rs_recovery = (RecoveryOldVars() ? 3 : 0);
                SetRefresh(RefreshFlags_RefreshAndImages);
            }, 1.0f);
        }, 0.2f);
    }
}

bool RocketStats::RecoveryOldVars()
{
    cvarManager->log("Recovery old vars !");

    CVarWrapper ors_session = cvarManager->getCvar("RS_session");
    if (!ors_session.IsNull())
        rs_mode = ors_session.getBoolValue();

    CVarWrapper ors_Use_v1 = cvarManager->getCvar("RS_Use_v1");
    CVarWrapper ors_Use_v2 = cvarManager->getCvar("RS_Use_v2");
    if (!ors_Use_v1.IsNull() && !ors_Use_v2.IsNull())
        SetTheme(ors_Use_v1.getBoolValue() ? "Default" : (ors_Use_v2.getBoolValue() ? "Redesigned" : "Arubinu42"));

    CVarWrapper ors_x_position = cvarManager->getCvar("RS_x_position");
    if (!ors_x_position.IsNull())
        rs_x = ors_x_position.getFloatValue();

    CVarWrapper ors_y_position = cvarManager->getCvar("RS_y_position");
    if (!ors_y_position.IsNull())
        rs_y = ors_y_position.getFloatValue();

    CVarWrapper ors_scale = cvarManager->getCvar("RS_scale");
    if (!ors_scale.IsNull())
        rs_scale = ors_scale.getFloatValue();

    CVarWrapper ors_disp_ig = cvarManager->getCvar("RS_disp_ig");
    if (!ors_disp_ig.IsNull())
        rs_disp_overlay = ors_disp_ig.getBoolValue();

    CVarWrapper ors_enable_float = cvarManager->getCvar("RS_enable_float");
    if (!ors_enable_float.IsNull())
        rs_enable_float = ors_enable_float.getBoolValue();

    CVarWrapper ors_hide_overlay_ig = cvarManager->getCvar("RS_hide_overlay_ig");
    if (!ors_hide_overlay_ig.IsNull())
        rs_enable_ingame = !ors_hide_overlay_ig.getBoolValue();

    CVarWrapper ors_disp_gamemode = cvarManager->getCvar("RS_disp_gamemode");
    if (!ors_disp_gamemode.IsNull())
        rs_hide_gm = !ors_disp_gamemode.getBoolValue();

    CVarWrapper ors_disp_rank = cvarManager->getCvar("RS_disp_rank");
    if (!ors_disp_rank.IsNull())
        rs_hide_rank = !ors_disp_rank.getBoolValue();

    CVarWrapper ors_disp_mmr = cvarManager->getCvar("RS_disp_mmr");
    if (!ors_disp_mmr.IsNull())
        rs_hide_mmr = !ors_disp_mmr.getBoolValue();

    CVarWrapper ors_disp_wins = cvarManager->getCvar("RS_disp_wins");
    if (!ors_disp_wins.IsNull())
        rs_hide_win = !ors_disp_wins.getBoolValue();

    CVarWrapper ors_disp_losses = cvarManager->getCvar("RS_disp_losses");
    if (!ors_disp_losses.IsNull())
        rs_hide_loss = !ors_disp_losses.getBoolValue();

    CVarWrapper ors_disp_streak = cvarManager->getCvar("RS_disp_streak");
    if (!ors_disp_streak.IsNull())
        rs_hide_streak = !ors_disp_streak.getBoolValue();

    CVarWrapper ors_stop_boost = cvarManager->getCvar("RocketStats_stop_boost");
    if (!ors_stop_boost.IsNull())
        rs_file_boost = ors_stop_boost.getBoolValue();

    ChangeTheme(rs_theme);
    WriteConfig();

    return !ors_session.IsNull();
}
#pragma endregion

void RocketStats::onLoad()
{
    // notifierToken = gameWrapper->GetMMRWrapper().RegisterMMRNotifier(std::bind(&RocketStats::UpdateMMR, this, std::placeholders::_1));

    // Retrieves the plugin version to display it in the menu
    if (exports.pluginVersion != nullptr)
        menu_version = ("v" + std::string(exports.pluginVersion));

    // Loads the language into the array and initializes the list of modes
    cvarManager->log(("Language: " + gameWrapper->GetUILanguage().ToString()).c_str());
    ChangeLang((gameWrapper->GetUILanguage().ToString() == "FRA") ? IDB_LANG_FRA : IDB_LANG_INT);
    modes = {
        GetLang(LANG_MODE_SESSION),
        GetLang(LANG_MODE_GAMEMODE),
        GetLang(LANG_MODE_ALWAYS),
        GetLang(LANG_MODE_ALWAYS_GAMEMODE)
    };

    // Define the operating folder, the "One Click" protocol
    SetDefaultFolder();
    SetCustomProtocol();

    // Loads important and rank images
    std::string logo_path = "RocketStats_images/logo.png";
    std::string title_path = "RocketStats_images/title.png";
    WriteResInFile(logo_path, IDB_LOGO, "PNG");
    WriteResInFile(title_path, IDB_TITLE, "PNG");
    rs_logo = LoadImg(logo_path);
    rs_title = LoadImg(title_path);
    LoadImgs();
    LoadThemes();

    // Initializes the different functionalities
    InitRank();
    InitStats();
    rs_recovery = (ReadConfig() ? 0 : 1);
    ChangeTheme(rs_theme);

    // Reset all files (and create them if they don't exist)
    ResetFiles();
    RemoveFile("RocketStats_Loose.txt"); // Delete the old file
    RemoveFile("RocketStats_images/BoostState.txt"); // Delete the old file
    RemoveFile("plugins/settings/rocketstats.set", true); // Delete the old file

    // Can be used from the console or in bindings
    cvarManager->registerNotifier("rs_toggle_menu", [this](std::vector<std::string> params) {
            ToggleSettings("rs_toggle_menu");
    }, GetLang(LANG_TOGGLE_MENU), PERMISSION_ALL);

    // Hook on Event
    gameWrapper->HookEvent("Function TAGame.GFxData_StartMenu_TA.EventTitleScreenClicked", std::bind(&RocketStats::ShowPlugin, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameViewportClient_TA.SetUIScaleModifier", std::bind(&RocketStats::UpdateUIScale, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function Engine.GameViewportClient.IsFullScreenViewport", std::bind(&RocketStats::UpdateUIScale, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&RocketStats::GameStart, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&RocketStats::GameEnd, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function CarComponent_Boost_TA.Active.BeginState", std::bind(&RocketStats::OnBoostStart, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function CarComponent_Boost_TA.Active.EndState", std::bind(&RocketStats::OnBoostEnd, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_TA.Destroyed", std::bind(&RocketStats::GameDestroyed, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GFxData_MenuStack_TA.PushMenu", std::bind([this]() { is_in_menu = true; }));
    gameWrapper->HookEvent("Function TAGame.GFxData_MenuStack_TA.PopMenu", std::bind([this]() { is_in_menu = false; }));

    gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatEvent", std::bind(&RocketStats::onStatEvent, this, std::placeholders::_1, std::placeholders::_2));
    gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", std::bind(&RocketStats::onStatTickerMessage, this, std::placeholders::_1, std::placeholders::_2));

    // Register Cvars
    if (rs_recovery)
    {
        cvarManager->registerCvar("RS_Use_v1", "0", "Use the v1 overlay", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_Use_v2", "0", "Use the v2 overlay", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_ig", "1", "Display information panel", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_hide_overlay_ig", "0", "Hide overlay while in-game", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_mmr", "1", "Display the current MMR", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_wins", "1", "Display the wins on the current game mode", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_losses", "1", "Display the losses on the current game mode", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_streak", "1", "Display the streak on the current game mode", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_rank", "1", "Display the rank on the current game mode", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_disp_gamemode", "1", "Display the current game mode", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_enable_float", "0", "Enable floating point for MMR (OBS only)", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_x_position", "0.700", "Overlay X position", true, true, 0, true, 1.0f);
        cvarManager->registerCvar("RS_y_position", "0.575", "Overlay Y position", true, true, 0, true, 1.0f);
        cvarManager->registerCvar("RS_scale", "1", "Overlay scale", true, true, 0, true, 10);
        cvarManager->registerCvar("RocketStats_stop_boost", "1", "Stop Boost animation", true, true, 0, true, 1);
        cvarManager->registerCvar("RS_session", "0", "Display session information instead of game mode", true, true, 0, true, 1, true);
    }

    cvarManager->registerCvar("cl_rocketstats_settings", (settings_open ? "1" : "0"), GetLang(LANG_TOGGLE_MENU_HELP), true, true, 0, true, 1, false).addOnValueChanged([this](std::string old, CVarWrapper now) {
        settings_open = now.getBoolValue();

        cvarManager->log("cl_rocketstats_settings: " + std::string(settings_open ? "true" : "false"));
        if (!settings_open)
            WriteConfig();
    });

    cvarManager->registerCvar("rs_mode", std::to_string(rs_mode), GetLang(LANG_MODE), true, true, 0, true, float(modes.size() - 1), false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_theme", std::to_string(rs_theme), GetLang(LANG_THEME), true, true, 0, false, 99, false).addOnValueChanged([this](std::string old, CVarWrapper now) {
        if (!ChangeTheme(now.getIntValue()))
            now.setValue(old);
    });

    cvarManager->registerCvar("rs_x", std::to_string(rs_x), GetLang(LANG_X), true, true, 0.f, true, 1.f, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_y", std::to_string(rs_y), GetLang(LANG_Y), true, true, 0.f, true, 1.f, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_scale", std::to_string(rs_scale), GetLang(LANG_SCALE), true, true, 0.001f, true, 10.f, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_rotate", std::to_string(rs_rotate), GetLang(LANG_ROTATE), true, true, -180.f, true, 180.f, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_opacity", std::to_string(rs_opacity), GetLang(LANG_OPACITY), true, true, 0.f, true, 1.f, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));

    cvarManager->registerCvar("rs_disp_overlay", (rs_disp_overlay ? "1" : "0"), GetLang(LANG_OVERLAY), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));

    cvarManager->registerCvar("rs_enable_inmenu", (rs_enable_inmenu ? "1" : "0"), GetLang(LANG_SHOW_IN_MENU), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_enable_ingame", (rs_enable_ingame ? "1" : "0"), GetLang(LANG_SHOW_IN_GAME), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshTheme, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_enable_float", (rs_enable_float ? "1" : "0"), GetLang(LANG_FLOATING_POINT), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_preview_rank", (rs_preview_rank ? "1" : "0"), GetLang(LANG_PREVIEW_RANK), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));

    cvarManager->registerCvar("rs_in_file", (rs_in_file ? "1" : "0"), GetLang(LANG_IN_FILE), true, true, 0, true, 1, true).addOnValueChanged([this](std::string old, CVarWrapper now) {
        UpdateFiles(true);
        RefreshTheme(old, now);
    });
    cvarManager->registerCvar("rs_file_gm", (rs_file_gm ? "1" : "0"), GetLang(LANG_FILE_GAMEMODE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_rank", (rs_file_rank ? "1" : "0"), GetLang(LANG_FILE_RANK), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_div", (rs_file_div ? "1" : "0"), GetLang(LANG_FILE_DIVISION), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_mmr", (rs_file_mmr ? "1" : "0"), GetLang(LANG_FILE_MMR), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_mmrc", (rs_file_mmrc ? "1" : "0"), GetLang(LANG_FILE_MMRCHANGE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_mmrcc", (rs_file_mmrcc ? "1" : "0"), GetLang(LANG_FILE_MMRCUMULCHANGE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_win", (rs_file_win ? "1" : "0"), GetLang(LANG_FILE_WINS), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_loss", (rs_file_loss ? "1" : "0"), GetLang(LANG_FILE_LOSSES), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_streak", (rs_file_streak ? "1" : "0"), GetLang(LANG_FILE_STREAKS), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_demo", (rs_file_demo ? "1" : "0"), GetLang(LANG_FILE_DEMOLITIONS), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_demom", (rs_file_demom ? "1" : "0"), GetLang(LANG_FILE_DEMOLITIONSMATCH), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_democ", (rs_file_democ ? "1" : "0"), GetLang(LANG_FILE_DEMOLITIONSCUMUL), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_death", (rs_file_death ? "1" : "0"), GetLang(LANG_FILE_DEATH), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_deathm", (rs_file_deathm ? "1" : "0"), GetLang(LANG_FILE_DEATHMATCH), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_deathc", (rs_file_deathc ? "1" : "0"), GetLang(LANG_FILE_DEATHCUMUL), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_file_boost", (rs_file_boost ? "1" : "0"), GetLang(LANG_FILE_BOOST), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));

    cvarManager->registerCvar("rs_hide_gm", (rs_hide_gm ? "1" : "0"), GetLang(LANG_HIDE_GAMEMODE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_rank", (rs_hide_rank ? "1" : "0"), GetLang(LANG_HIDE_RANK), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_div", (rs_hide_div ? "1" : "0"), GetLang(LANG_HIDE_DIVISION), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_mmr", (rs_hide_mmr ? "1" : "0"), GetLang(LANG_HIDE_MMR), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_mmrc", (rs_hide_mmrc ? "1" : "0"), GetLang(LANG_HIDE_MMRCHANGE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_mmrcc", (rs_hide_mmrcc ? "1" : "0"), GetLang(LANG_HIDE_MMRCUMULCHANGE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_win", (rs_hide_win ? "1" : "0"), GetLang(LANG_HIDE_WINS), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_loss", (rs_hide_loss ? "1" : "0"), GetLang(LANG_HIDE_LOSSES), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_streak", (rs_hide_streak ? "1" : "0"), GetLang(LANG_HIDE_STREAKS), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_demo", (rs_hide_demo ? "1" : "0"), GetLang(LANG_HIDE_DEMOLITIONS), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_demom", (rs_hide_demom ? "1" : "0"), GetLang(LANG_HIDE_DEMOLITIONSMATCH), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_democ", (rs_hide_democ ? "1" : "0"), GetLang(LANG_HIDE_DEMOLITIONSCUMUL), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_death", (rs_hide_death ? "1" : "0"), GetLang(LANG_HIDE_DEATH), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_deathm", (rs_hide_deathm ? "1" : "0"), GetLang(LANG_HIDE_DEATHMATCH), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_hide_deathc", (rs_hide_deathc ? "1" : "0"), GetLang(LANG_HIDE_DEATHCUMUL), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_replace_mmr", (rs_replace_mmr ? "1" : "0"), GetLang(LANG_MMR_TO_MMRCHANGE), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));
    cvarManager->registerCvar("rs_replace_mmrc", (rs_replace_mmrc ? "1" : "0"), GetLang(LANG_MMRCHANGE_TO_MMR), true, true, 0, true, 1, false).addOnValueChanged(std::bind(&RocketStats::RefreshFiles, this, std::placeholders::_1, std::placeholders::_2));

    // Displays the plugin shortly after initialization
    gameWrapper->SetTimeout([&](GameWrapper* gameWrapper) {
        if (rs_recovery)
        {
            std::string path = "RocketStats_images/welcome.png";
            if (WriteResInFile(path, ((gameWrapper->GetUILanguage().ToString() == "FRA") ? IDB_WEL_FRA : IDB_WEL_INT), "PNG"))
                rs_welcome = LoadImg(path);
        }

        UpdateUIScale("onLoad");
        TogglePlugin("onLoad", ToggleFlags_Show);
    }, 0.2f);
}

void RocketStats::onUnload()
{
    WriteConfig(); // Save settings (if not already done)

    gameWrapper->UnhookEvent("Function TAGame.GFxData_StartMenu_TA.EventTitleScreenClicked");
    gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
    gameWrapper->UnhookEvent("Function CarComponent_Boost_TA.Active.BeginState");
    gameWrapper->UnhookEvent("Function CarComponent_Boost_TA.Active.EndState");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.Destroyed");

    //gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatEvent");
    //gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");

    TogglePlugin("onUnload", ToggleFlags_Hide); // Hide the plugin before unloading it
}

void RocketStats::SetDefaultFolder()
{
    // If the old folder exist, copy everything to the new path
    if (ExistsPath("RocketStats", true))
    {
        std::string old_path = GetPath("RocketStats", true);
        fs::copy(old_path, GetPath(), (fs::copy_options::recursive | fs::copy_options::update_existing));
        fs::remove_all(old_path);
    }
}

void RocketStats::SetCustomProtocol()
{
    std::string dir = GetPath();
    Utils::ReplaceAll(dir, "/", "\\");

    HKEY key;
    LPCTSTR value;
    DWORD dwDisposition;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\RocketStats"), 0, NULL, 0, KEY_WRITE, NULL, &key, &dwDisposition) == ERROR_SUCCESS)
    {
        value = TEXT("URL:RocketStats protocol");
        if (RegSetValueEx(key, TEXT(""), 0, REG_SZ, (const BYTE*)value, sizeof(TCHAR) * (lstrlen(value) + 1)) != ERROR_SUCCESS)
            cvarManager->log("Set protocol failed!");

        value = TEXT("RocketStats");
        if (RegSetValueEx(key, TEXT("URL Protocol"), 0, REG_SZ, (const BYTE*)value, sizeof(TCHAR) * (lstrlen(value) + 1)) != ERROR_SUCCESS)
            cvarManager->log("Set url failed!");

        RegCloseKey(key);

        /*if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\RocketStats\\DefaultIcon"), 0, NULL, 0, KEY_WRITE, NULL, &key, &dwDisposition) == ERROR_SUCCESS)
        {
            value = Utils::ConvertToLPSTR(installer + ", 1");
            if (RegSetValueEx(key, TEXT(""), 0, REG_SZ, (const BYTE*)value, sizeof(TCHAR) * (lstrlen(value) + 1)) != ERROR_SUCCESS)
                cvarManager->log("Set icon failed!");

            RegCloseKey(key);
        }*/

        if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\RocketStats\\shell\\open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &key, &dwDisposition) == ERROR_SUCCESS)
        {
            value = Utils::ConvertToLPSTR("rundll32 shell32.dll,ShellExec_RunDLL \"powershell\" \"try { $theme = \\\"%1\\\".split( '/' )[ 3 ]; $version = \\\"%1\\\".split( '/' )[ 4 ]; $dir = \\\"" + dir + "\"; $url = \\\"http://rocketstats.net/$theme/dl/$version\\\"; $zip = \\\"$dir\\toinstall.zip\\\"; $dest = \\\"$dir\\RocketStats_themes\\\"; if ( \\\"%1\\\".split( '/' )[ 2 ] -eq 'install' -and $theme -match '^\\d+$' -and $version -match '^\\d+$' ) { echo 'Theme being downloaded ...'; Invoke-WebRequest -Uri $url -OutFile $zip >$null 2>$null; try { echo 'Theme being installed ...'; Expand-Archive $zip -DestinationPath $dest -Force >$null 2>$null; echo 'Theme successfully installed.'; echo 'Click ''R'' to the right of the ''Reload Theme'' button in RocketStats to add the theme to the list.'; Remove-Item $zip >$null 2>$null; } catch { echo 'Install error!'; } } else { echo 'The requested theme could not be found!'; } } catch { echo 'Download error!'; } echo ''; echo 'Press Enter to continue ...'; Read-Host;\"");
            if (RegSetValueEx(key, TEXT(""), 0, REG_SZ, (const BYTE*)value, sizeof(TCHAR) * (lstrlen(value) + 1)) != ERROR_SUCCESS)
                cvarManager->log("Set command failed!");

            RegCloseKey(key);
        }
    }
}

void RocketStats::ShowPlugin(std::string eventName)
{
    TogglePlugin(eventName, ToggleFlags_Show);
}

void RocketStats::UpdateUIScale(std::string eventName)
{
    rs_screen_scale[0] = gameWrapper->GetInterfaceScale();
    rs_screen_scale[1] = gameWrapper->GetDisplayScale();
    cvarManager->log("Scale: " + std::to_string(rs_screen_scale[0]) + " " + std::to_string(rs_screen_scale[1]));
    SetRefresh(RefreshFlags_Refresh);
}

void RocketStats::TogglePlugin(std::string eventName, ToggleFlags mode)
{
    if (mode == ToggleFlags_Toggle || (mode == ToggleFlags_Show && !plugin_open) || (mode == ToggleFlags_Hide && plugin_open))
    {
        plugin_open = !plugin_open;
        cvarManager->executeCommand("togglemenu " + GetMenuName());
    }
}

void RocketStats::ToggleSettings(std::string eventName, ToggleFlags mode)
{
    if (mode == ToggleFlags_Toggle || (mode == ToggleFlags_Show && !settings_open) || (mode == ToggleFlags_Hide && settings_open))
    {
        settings_open = !settings_open;
        cvarManager->getCvar("cl_rocketstats_settings").setValue(settings_open);

        if (!settings_open)
            WriteConfig(); // Saves settings when closing the menu
    }
}

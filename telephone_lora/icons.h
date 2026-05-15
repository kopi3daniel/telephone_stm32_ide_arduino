/**

 * Ce fichier contient TOUTES les icones utilisees dans l interface
 * sous forme de bitmaps 8x8 et 16x16 pixels.
 * 
 * Organisation :
 * - Icones de la barre d etat (8x8)
 * - Icones de notification (8x8)
 * - Icones du menu principal (16x16)
 * - Icones d action (16x16)
 * - Icones diverses (8x8 et 16x16)
 */

#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>
#include "config.h"

class Icons {
    
public:
    // ============================================
    // DIMENSIONS DES ICONES
    // ============================================
    static const uint8_t ICON_SMALL  = 8;
    static const uint8_t ICON_MEDIUM = 16;
    static const uint8_t ICON_LARGE  = 32;
    
    // ============================================
    // ICONES 8x8 - SIGNAL LORA
    // ============================================
    static const uint8_t signal_none[8];
    static const uint8_t signal_low[8];
    static const uint8_t signal_medium[8];
    static const uint8_t signal_good[8];
    static const uint8_t signal_excellent[8];
    
    // ============================================
    // ICONES 8x8 - BATTERIE
    // ============================================
    static const uint8_t battery_full[8];
    static const uint8_t battery_75[8];
    static const uint8_t battery_50[8];
    static const uint8_t battery_25[8];
    static const uint8_t battery_empty[8];
    static const uint8_t battery_charging[8];
    
    // ============================================
    // ICONES 8x8 - NOTIFICATIONS
    // ============================================
    static const uint8_t notif_message[8];
    static const uint8_t notif_call_missed[8];
    static const uint8_t notif_silent[8];
    static const uint8_t notif_vibrate[8];
    static const uint8_t notif_alarm[8];
    static const uint8_t notif_torch[8];
    static const uint8_t notif_lock[8];
    static const uint8_t notif_relay[8];
    
    // ============================================
    // ICONES 8x8 - ACTIONS
    // ============================================
    static const uint8_t action_call[8];
    static const uint8_t action_end_call[8];
    static const uint8_t action_mute[8];
    static const uint8_t action_speaker[8];
    static const uint8_t action_contacts[8];
    static const uint8_t action_settings[8];
    static const uint8_t action_search[8];
    static const uint8_t action_delete[8];
    static const uint8_t action_edit[8];
    static const uint8_t action_add[8];
    static const uint8_t action_check[8];
    static const uint8_t action_cross[8];
    static const uint8_t action_arrow_up[8];
    static const uint8_t action_arrow_down[8];
    static const uint8_t action_arrow_left[8];
    static const uint8_t action_arrow_right[8];
    static const uint8_t action_back[8];
    static const uint8_t action_menu[8];
    static const uint8_t action_power[8];
    static const uint8_t action_info[8];
    
    // ============================================
    // ICONES 16x16 - MENU PRINCIPAL
    // ============================================
    static const uint8_t menu_calls[32];
    static const uint8_t menu_messages[32];
    static const uint8_t menu_contacts[32];
    static const uint8_t menu_settings[32];
    
    // ============================================
    // ICONES 16x16 - APPELS
    // ============================================
    static const uint8_t call_incoming[32];
    static const uint8_t call_outgoing[32];
    static const uint8_t call_missed[32];
    static const uint8_t call_active[32];
    
    // ============================================
    // ICONES 16x16 - DIVERS
    // ============================================
    static const uint8_t icon_phone[32];
    static const uint8_t icon_sms[32];
    static const uint8_t icon_contact[32];
    static const uint8_t icon_keypad[32];
    static const uint8_t icon_clock[32];
    static const uint8_t icon_shield[32];
    static const uint8_t icon_network[32];
    static const uint8_t icon_bluetooth[32];
    static const uint8_t icon_gps[32];
    
    // ============================================
    // METHODES UTILITAIRES
    // ============================================
    
    static const uint8_t* getSmallIcon(const char* name);
    static const uint8_t* getMediumIcon(const char* name);
    static const uint8_t* getSignalIcon(int8_t rssi);
    static const uint8_t* getBatteryIcon(uint8_t percent, bool charging);
    static const uint8_t* getCallTypeIcon(uint8_t callType);
};

#endif // ICONS_H
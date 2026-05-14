/**
 * ---------------------------------------------------------------------------
 * menu.cpp - Implementation du widget de menu
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "menu.h"
#include "display.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern Display display;

// ============================================
// CONSTRUCTEUR
// ============================================
Menu::Menu() {
    itemCount = 0;
    selectedIndex = 0;
    scrollOffset = 0;
    visibleCount = MENU_VISIBLE_ITEMS;
    
    menuX = 0;
    menuY = 10;
    menuWidth = SCREEN_WIDTH;
    menuHeight = SCREEN_HEIGHT - menuY - 10;
    
    memset(title, 0, sizeof(title));
    showTitle = true;
    showScrollbar = true;
    
    isActive = false;
    needsRedraw = true;
    
    memset(items, 0, sizeof(items));
}

void Menu::init(void) {
    clearAll();
    selectedIndex = 0;
    scrollOffset = 0;
    needsRedraw = true;
}

// ============================================
// CONFIGURATION
// ============================================
void Menu::setBounds(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    menuX = x;
    menuY = y;
    menuWidth = width;
    menuHeight = height;
    if (MENU_ITEM_HEIGHT > 0) {
        visibleCount = height / MENU_ITEM_HEIGHT;
    }
}

void Menu::setTitle(const char* menuTitle) {
    if (menuTitle) {
        strncpy(title, menuTitle, 31);
        title[31] = '\0';
    }
    needsRedraw = true;
}

void Menu::showTitleBar(bool show) {
    showTitle = show;
    needsRedraw = true;
}

void Menu::showScrollBar(bool show) {
    showScrollbar = show;
    needsRedraw = true;
}

// ============================================
// AJOUT D ELEMENTS
// ============================================
void Menu::addItem(const char* label, MenuItemType_t type) {
    if (itemCount >= MENU_MAX_ITEMS || label == NULL) return;
    
    MenuItem_t* item = &items[itemCount];
    item->label = label;
    item->sublabel = NULL;
    item->value = NULL;
    item->type = type;
    item->iconIndex = 0;
    item->enabled = true;
    item->toggleState = false;
    item->sliderValue = 0;
    item->sliderMax = 100;
    item->action = NULL;
    
    itemCount++;
    needsRedraw = true;
}

void Menu::addItemWithSub(const char* label, const char* sublabel) {
    if (itemCount >= MENU_MAX_ITEMS) return;
    
    items[itemCount].sublabel = sublabel;
    addItem(label, MENU_ITEM_NORMAL);
}

void Menu::addItemWithValue(const char* label, const char* value) {
    if (itemCount >= MENU_MAX_ITEMS) return;
    
    items[itemCount].value = value;
    addItem(label, MENU_ITEM_NORMAL);
}

void Menu::addToggle(const char* label, bool initialState) {
    if (itemCount >= MENU_MAX_ITEMS) return;
    
    MenuItem_t* item = &items[itemCount];
    item->type = MENU_ITEM_TOGGLE;
    item->toggleState = initialState;
    addItem(label, MENU_ITEM_TOGGLE);
}

void Menu::addSlider(const char* label, uint8_t value, uint8_t maxVal) {
    if (itemCount >= MENU_MAX_ITEMS) return;
    
    MenuItem_t* item = &items[itemCount];
    item->type = MENU_ITEM_SLIDER;
    item->sliderValue = value;
    item->sliderMax = maxVal;
    addItem(label, MENU_ITEM_SLIDER);
}

void Menu::addAction(const char* label, void (*action)(void)) {
    if (itemCount >= MENU_MAX_ITEMS) return;
    
    MenuItem_t* item = &items[itemCount];
    item->type = MENU_ITEM_ACTION;
    item->action = action;
    addItem(label, MENU_ITEM_ACTION);
}

void Menu::addSeparator(void) {
    if (itemCount >= MENU_MAX_ITEMS) return;
    
    items[itemCount].type = MENU_ITEM_SEPARATOR;
    items[itemCount].label = "---";
    itemCount++;
    needsRedraw = true;
}

void Menu::clearAll(void) {
    memset(items, 0, sizeof(items));
    itemCount = 0;
    selectedIndex = 0;
    scrollOffset = 0;
    needsRedraw = true;
}

uint8_t Menu::getItemCount(void) { return itemCount; }

// ============================================
// NAVIGATION
// ============================================
void Menu::moveUp(void) {
    if (selectedIndex > 0) {
        do {
            selectedIndex--;
        } while (selectedIndex > 0 && 
                (items[selectedIndex].type == MENU_ITEM_SEPARATOR || 
                 !items[selectedIndex].enabled));
        
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
        needsRedraw = true;
    }
}

void Menu::moveDown(void) {
    if (selectedIndex < itemCount - 1) {
        do {
            selectedIndex++;
        } while (selectedIndex < itemCount - 1 && 
                (items[selectedIndex].type == MENU_ITEM_SEPARATOR || 
                 !items[selectedIndex].enabled));
        
        if (selectedIndex >= scrollOffset + visibleCount) {
            scrollOffset = selectedIndex - visibleCount + 1;
        }
        needsRedraw = true;
    }
}

void Menu::selectItem(uint8_t index) {
    if (index < itemCount && items[index].type != MENU_ITEM_SEPARATOR) {
        selectedIndex = index;
        
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
        if (selectedIndex >= scrollOffset + visibleCount) {
            scrollOffset = selectedIndex - visibleCount + 1;
        }
        needsRedraw = true;
    }
}

void Menu::activateSelected(void) {
    if (selectedIndex < itemCount) {
        MenuItem_t* item = &items[selectedIndex];
        
        if (item->type == MENU_ITEM_ACTION && item->action != NULL) {
            item->action();
        }
    }
}

uint8_t Menu::getSelectedIndex(void) { return selectedIndex; }
MenuItemType_t Menu::getSelectedType(void) {
    if (selectedIndex < itemCount) {
        return items[selectedIndex].type;
    }
    return MENU_ITEM_NORMAL;
}

// ============================================
// MANIPULATION DES ELEMENTS
// ============================================
void Menu::setItemEnabled(uint8_t index, bool enabled) {
    if (index < itemCount) {
        items[index].enabled = enabled;
        needsRedraw = true;
    }
}

void Menu::setItemLabel(uint8_t index, const char* label) {
    if (index < itemCount && label) {
        items[index].label = label;
        needsRedraw = true;
    }
}

void Menu::setItemValue(uint8_t index, const char* value) {
    if (index < itemCount) {
        items[index].value = value;
        needsRedraw = true;
    }
}

void Menu::toggleItem(uint8_t index) {
    if (index < itemCount && items[index].type == MENU_ITEM_TOGGLE) {
        items[index].toggleState = !items[index].toggleState;
        needsRedraw = true;
    }
}

void Menu::setSliderValue(uint8_t index, uint8_t value) {
    if (index < itemCount && items[index].type == MENU_ITEM_SLIDER) {
        items[index].sliderValue = value;
        if (items[index].sliderValue > items[index].sliderMax) {
            items[index].sliderValue = items[index].sliderMax;
        }
        needsRedraw = true;
    }
}

bool Menu::getToggleState(uint8_t index) {
    if (index < itemCount) return items[index].toggleState;
    return false;
}

uint8_t Menu::getSliderValue(uint8_t index) {
    if (index < itemCount) return items[index].sliderValue;
    return 0;
}

// ============================================
// AFFICHAGE
// ============================================
void Menu::draw(void) {
    if (showTitle && strlen(title) > 0) {
        drawTitle();
    }
    
    for (uint8_t i = 0; i < visibleCount; i++) {
        uint8_t itemIdx = scrollOffset + i;
        if (itemIdx >= itemCount) break;
        
        uint8_t drawY = menuY + (i * MENU_ITEM_HEIGHT);
        bool selected = (itemIdx == selectedIndex);
        drawItem(itemIdx, drawY, selected);
    }
    
    if (showScrollbar && itemCount > visibleCount) {
        drawScrollbar();
    }
    
    needsRedraw = false;
}

void Menu::drawItem(uint8_t index, uint8_t y, bool selected) {
    if (index >= itemCount) return;
    
    MenuItem_t* item = &items[index];
    
    if (y < menuY || y > menuY + menuHeight) return;
    
    if (item->type == MENU_ITEM_SEPARATOR) {
        display.drawLine(menuX + 2, y + 4, menuX + menuWidth - 4, y + 4);
        return;
    }
    
    if (selected && item->enabled) {
        display.drawRect(menuX, y, menuWidth, MENU_ITEM_HEIGHT, true);
    }
    
    uint8_t textX = menuX + 4;
    uint8_t textY = y + 1;
    
    if (item->iconIndex > 0) {
        drawItemIcon(index, textX, textY);
        textX += 10;
    }
    
    char displayText[40];
    strncpy(displayText, item->label, 35);
    displayText[35] = '\0';
    
    if (item->type == MENU_ITEM_SUBMENU) {
        strcat(displayText, " >");
    }
    
    if (selected && item->enabled) {
        display.drawInvertedText(textX, textY, displayText, 1);
    } else {
        display.drawText(textX, textY, displayText, 1);
    }
    
    if (item->sublabel != NULL) {
        uint8_t subY = y + 1;
        if (selected && item->enabled) {
            display.drawInvertedText(textX + 60, subY, item->sublabel, 1);
        } else {
            display.drawText(textX + 60, subY, item->sublabel, 1);
        }
    }
    
    drawItemValue(index, menuX + menuWidth - 40, textY);
}

void Menu::drawItemValue(uint8_t index, uint8_t x, uint8_t y) {
    if (index >= itemCount) return;
    
    MenuItem_t* item = &items[index];
    bool selected = (index == selectedIndex);
    
    switch (item->type) {
        case MENU_ITEM_TOGGLE:
            drawToggle(x, y, item->toggleState);
            break;
            
        case MENU_ITEM_SLIDER:
            drawSlider(x, y, item->sliderValue, item->sliderMax);
            break;
            
        case MENU_ITEM_NORMAL:
            if (item->value != NULL) {
                if (selected && item->enabled) {
                    display.drawInvertedText(x, y, item->value, 1);
                } else {
                    display.drawText(x, y, item->value, 1);
                }
            }
            break;
            
        default:
            break;
    }
}

void Menu::drawToggle(uint8_t x, uint8_t y, bool state) {
    display.drawRect(x, y, 20, 7, false);
    
    if (state) {
        display.drawRect(x + 10, y, 10, 7, true);
        display.drawText(x + 2, y, "ON", 1);
    } else {
        display.drawRect(x, y, 10, 7, true);
        display.drawText(x + 12, y, "OFF", 1);
    }
}

void Menu::drawSlider(uint8_t x, uint8_t y, uint8_t value, uint8_t maxVal) {
    uint8_t barWidth = 30;
    
    display.drawRect(x, y + 2, barWidth, 4, false);
    
    if (maxVal > 0) {
        uint8_t filled = (uint16_t)(value * barWidth) / maxVal;
        if (filled > 0) {
            display.drawRect(x, y + 2, filled, 4, true);
        }
    }
    
    char valStr[5];
    snprintf(valStr, sizeof(valStr), "%d", value);
    display.drawText(x + barWidth + 2, y, valStr, 1);
}

void Menu::drawTitle(void) {
    display.drawCenteredText(menuY - 10, title, 1);
    display.drawLine(menuX, menuY - 1, menuX + menuWidth, menuY - 1);
}

void Menu::drawScrollbar(void) {
    if (itemCount == 0) return;
    
    uint8_t barHeight = (visibleCount * menuHeight) / itemCount;
    if (barHeight < 4) barHeight = 4;
    
    uint8_t barY = menuY + (scrollOffset * menuHeight) / itemCount;
    
    display.drawRect(menuX + menuWidth - 2, barY, 
                    MENU_SCROLLBAR_WIDTH, barHeight, true);
}

void Menu::drawItemIcon(uint8_t index, uint8_t x, uint8_t y) {
    if (index >= itemCount) return;
    // Les icones seraient dessinees ici selon items[index].iconIndex
}

// ============================================
// ETAT
// ============================================
void Menu::setActive(bool active) {
    isActive = active;
    if (active) needsRedraw = true;
}

void Menu::forceRedraw(void) {
    needsRedraw = true;
}

bool Menu::isActiveMenu(void) {
    return isActive;
}

// ============================================
// FIN DU FICHIER menu.cpp
// ============================================
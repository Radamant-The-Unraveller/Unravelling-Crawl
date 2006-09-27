#ifndef __MENU_H__
#define __MENU_H__

#include <string>
#include <vector>
#include <algorithm>
#include "AppHdr.h"
#include "externs.h"
#include "defines.h"
#include "libutil.h"

enum MenuEntryLevel
{
    MEL_NONE = -1,
    MEL_TITLE,
    MEL_SUBTITLE,
    MEL_ITEM
};

struct menu_letter
{
    char letter;

    menu_letter() : letter('a') { }
    menu_letter(char c) : letter(c) { }

    operator char () const { return letter; }
    
    const menu_letter &operator ++ ()
    {
        letter = letter == 'z'? 'A' :
                 letter == 'Z'? 'a' :
                                letter + 1;
        return *this;
    }

    // dummy postfix argument unnamed to stop gcc from complaining
    menu_letter operator ++ (int)
    {
        menu_letter copy = *this;
        this->operator++();
        return copy;
    }
};

struct item_def;
struct MenuEntry
{
    std::string text;
    int quantity, selected_qty;
    int colour;
    std::vector<int> hotkeys;
    MenuEntryLevel level;
    void *data;

    MenuEntry( const std::string &txt = std::string(""),
               MenuEntryLevel lev = MEL_ITEM,
               int qty = 0,
               int hotk = 0 ) :
        text(txt), quantity(qty), selected_qty(0), colour(-1),
        hotkeys(), level(lev), data(NULL)
    {
        colour = lev == MEL_ITEM?       LIGHTGREY :
                 lev == MEL_SUBTITLE?   BLUE  :
                                        WHITE;
        if (hotk)
            hotkeys.push_back( hotk );
    }
    virtual ~MenuEntry() { }

    bool operator<( const MenuEntry& rhs ) const {
	return text < rhs.text;
    }

    void add_hotkey( int key )
    {
        if (key && !is_hotkey(key))
            hotkeys.push_back( key );
    }
    
    bool is_hotkey( int key ) const
    {
        return find( hotkeys.begin(), hotkeys.end(), key ) != hotkeys.end();
    }

    bool is_primary_hotkey( int key ) const
    {
        return hotkeys.size()? hotkeys[0] == key : false;
    }
    
    virtual std::string get_text() const
    {
        if (level == MEL_ITEM && hotkeys.size())
        {
            char buf[300];
            snprintf(buf, sizeof buf,
                    "%c - %s", hotkeys[0], text.c_str());
            return std::string(buf);
        }
        return std::string(level == MEL_SUBTITLE? " " :
                level == MEL_ITEM? "" : "  ") + text;
    }

    virtual bool selected() const
    {
        return selected_qty > 0 && quantity;
    }

    void select( int qty = -1 )
    {
        if (selected())
            selected_qty = 0;
        else if (quantity)
            selected_qty = qty == -1? quantity : qty;
    }
};

class MenuHighlighter
{
public:
    virtual int entry_colour(const MenuEntry *entry) const;
    virtual ~MenuHighlighter() { }
};

enum MenuFlag
{
    MF_NOSELECT         = 0x0000,   // No selection is permitted
    MF_SINGLESELECT     = 0x0001,   // Select just one item
    MF_MULTISELECT      = 0x0002,   // Select multiple items
    MF_NO_SELECT_QTY    = 0x0004,   // Disallow partial selections
    MF_ANYPRINTABLE     = 0x0008,   // Any printable character is valid, and 
                                    // closes the menu.
    MF_SELECT_ANY_PAGE  = 0x0010,   // Allow selections to occur on any page.

    MF_ALWAYS_SHOW_MORE = 0x0020,   // Always show the -more- footer
    MF_NOWRAP           = 0x0040,   // Paging past the end will not wrap back.

    MF_EASY_EXIT        = 0x1000
};

///////////////////////////////////////////////////////////////////////
// NOTE
// As a general contract, any pointers you pass to Menu methods are OWNED BY
// THE MENU, and will be deleted by the Menu on destruction. So any pointers
// you pass in MUST be allocated with new, or Crawl will crash.

#define NUMBUFSIZ 10
class Menu
{
public:
    Menu( int flags = MF_MULTISELECT );
    virtual ~Menu();

    // Sets menu flags to new_flags. If use_options is true, game options may
    // override options.
    void set_flags(int new_flags, bool use_options = true);
    int  get_flags() const        { return flags; }
    bool is_set( int flag ) const;
    
    bool draw_title_suffix( const std::string &s, bool titlefirst = true );
    void update_title();

    // Sets a replacement for the --more-- string.
    void set_more(const formatted_string &more);
    
    void set_highlighter( MenuHighlighter *h );
    void set_title( MenuEntry *e );
    void add_entry( MenuEntry *entry );
    void get_selected( std::vector<MenuEntry*> *sel ) const;

    void set_select_filter( std::vector<text_pattern> filter )
    {
        select_filter = filter;
    }

    unsigned char getkey() const { return lastch; }

    void reset();
    std::vector<MenuEntry *> show();

public:
    typedef std::string (*selitem_tfn)( const std::vector<MenuEntry*> *sel );
    typedef void (*drawitem_tfn)(int index, const MenuEntry *me);
    typedef int (*keyfilter_tfn)(int keyin);

    selitem_tfn      f_selitem;
    drawitem_tfn     f_drawitem;
    keyfilter_tfn    f_keyfilter;

protected:
    MenuEntry *title;
    int flags;
    
    int first_entry, y_offset;
    int pagesize;

    formatted_string more;

    std::vector<MenuEntry*>  items;
    std::vector<MenuEntry*>  *sel;
    std::vector<text_pattern> select_filter;

    // Class that is queried to colour menu entries.
    MenuHighlighter *highlighter;

    int num;

    unsigned char lastch;

    bool alive;

    void do_menu( std::vector<MenuEntry*> *selected );
    virtual void draw_select_count( int count );
    virtual void draw_item( int index ) const;
    virtual void draw_item(int index, const MenuEntry *me) const;

    virtual void draw_title();
    void draw_menu( std::vector<MenuEntry*> *selected );
    bool page_down();
    bool line_down();
    bool page_up();
    bool line_up();

    bool in_page(int index) const;

    void deselect_all(bool update_view = true);
    void select_items( int key, int qty = -1 );
    void select_index( int index, int qty = -1 );

    bool is_hotkey(int index, int key );
    bool is_selectable(int index) const;

    int item_colour(const MenuEntry *me) const;
    
    virtual bool process_key( int keyin );
};

int menu_colour(const std::string &itemtext);

#endif

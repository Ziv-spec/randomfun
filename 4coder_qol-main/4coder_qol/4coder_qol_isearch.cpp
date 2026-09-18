
// TODO(BYP): regex

// TODO(ziv): Search Improvement
//
// Single result mode 
// Empty string search should begin with last search (will not begin with jump), if I write anything, it clears the search and follows normal search produedure 
// if on the other hand I am on a token/word then it should begin with the token as a search, no jump, if I write anything it clears the search and follows normal search procedure
//
// Allow Tab to autocomplete , Shift Tab to autocomplete backwards
// Allow word highlighting with Shift + Left/Right, deleting characters from middle & adding
//
// Multi result mode
// When Shift Down/Up switch to all matches highlight & allow multicursor mode to work nicely
// Alt + C will change case sensitive / insensetive search results 
//
// 


// TODO(ziv):
// fix the way alphaneumeric movement works in 'zk_move_alphaneumeric_boundry' for both ways
// add autocomplete
// add highlighting for search results
// handle cursor upport
// multi cursor support 
// have support for search in all files and so on.. look into #showcase in discord
//  simplify drawing logic in 'SEARCH_render_search_bar'
// also have a fuzzy search character like get_*_pos will give me everything that matches those (make search better :)

// I need to specify the hardening and softening of search result as a filter. 
// so for exmaple Alt+c will harden everything

// Autocomplete



//

CUSTOM_ID(attachment, view_search_bar);
CUSTOM_ID(attachment, view_search_multi_cursor_highlights);
CUSTOM_ID(attachment, view_search_all_matches_highlights);

struct Search_Bar {
    String_Const_u8 prompt; 
    String_Const_u8 string; 
    
    b32 is_selection_active;
    i64 anchor_pos; // when selecting, this is the position selection
    // is anchord around when the cursor is moving (same as a marker)
    i64 cursor_pos; // cursor position in the search bar
    
    u64 string_capacity; // TODO(ziv): figure out why is this needed
};

function b32 set_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar) {
    Managed_Scope scope = view_get_managed_scope(app, view);
    Search_Bar **sb = scope_attachment(app, scope, view_search_bar, Search_Bar *);
    if (sb == NULL) return false;
    // Set search bar data
    *sb = bar;
    return true;
}

function b32 get_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar) {
    Assert(bar != NULL); 
    Managed_Scope scope = view_get_managed_scope(app, view);
    Search_Bar **sb = scope_attachment(app, scope, view_search_bar, Search_Bar *);
    if (*sb == NULL) return false;
    *bar = **sb;
    return true;
}

inline function Range_i64 get_selection_range(Search_Bar bar) {
    return Ii64(bar.cursor_pos, bar.anchor_pos);
}


//-





//- Declarations 
// Used to render improved query bar functionality, and highlight all matches
// function Rect_f32 SEARCH_render_query_bar(Application_Links *app, Rect_f32 region, View_ID view, Face_ID face_id);
// Make sure search highlights are rendered with low opacity to not be too overbearing for me
function void SEARCH_render_search_highlights(View_ID view);

// 


// Implementations

//- Hooks 

function Rect_f32 SEARCH_render_search_bar(Application_Links *app, Rect_f32 region, View_ID view, Face_ID face_id) {
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    
    Search_Bar search_bar; 
    if (get_active_search_bar(app, view, &search_bar)) {
        Rect_f32_Pair pair = layout_query_bar_on_bot(region, line_height, 1);
        
        Rect_f32 bar_rect = pair.max;
        
        Scratch_Block scratch(app);
        Fancy_Line list = {};
        push_fancy_string(scratch, &list, fcolor_id(defcolor_pop1),         search_bar.prompt);
        f32 prompt_width = get_fancy_line_width(app, face_id, &list); 
        push_fancy_string(scratch, &list, fcolor_id(defcolor_text_default), search_bar.string);
        
        Vec2_f32 p = bar_rect.p0 + V2f32(2.f, 2.f);
        
        // Find out exact cursor position (to draw cursor)
        f32 cursor_x_pos = 0;
        f32 anchor_x_pos = 0;
        {
            String_Const_u8 string_up_to_cursor = { 
                search_bar.string.str, 
                (u64)search_bar.cursor_pos
            };
            
            Fancy_Line cp_list = {};
            push_fancy_string(scratch, &cp_list, fcolor_id(defcolor_pop1),
                              string_up_to_cursor);
            cursor_x_pos = p.x + prompt_width + get_fancy_line_width(app, face_id, &cp_list); 
            
            
            String_Const_u8 string_up_to_anchor = { 
                search_bar.string.str, 
                (u64)search_bar.anchor_pos
            };
            
            Fancy_Line achr_list = {};
            push_fancy_string(scratch, &achr_list, fcolor_id(defcolor_pop1),
                              string_up_to_anchor);
            anchor_x_pos = p.x + prompt_width + get_fancy_line_width(app, face_id, &achr_list); 
        }
        
        
        // Draw selection
        if (search_bar.is_selection_active) {
            
            Range_f32 sel = If32(anchor_x_pos, cursor_x_pos);
            draw_rectangle_fcolor(app, Rf32_xy_wh(sel.min, p.y, sel.max - sel.min, face_metrics.line_height), 0.f, fcolor_id(defcolor_at_highlight, 0));
            
        }
        
        // Draw text
        p = draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
        
        // Draw cursor rect
        draw_rectangle_fcolor(app, Rf32_xy_wh(cursor_x_pos, p.y, 2.f, face_metrics.line_height), 0.f, fcolor_id(defcolor_cursor, 0));
        
        region = pair.min;
    }
    
    return(region);
}

//- Helpers 

function String_Const_u8_Array
kv_string_split_wildcards(Arena *arena, String_Const_u8 string)
{
    String_Const_u8_Array array = {};
    List_String_Const_u8 list = string_split(arena, string, (u8*)"* ", 2);
    array.count   = list.node_count;
    array.strings = push_array(arena, String_Const_u8, array.count);
    i64 index = 0;
    for (Node_String_Const_u8 *node = list.first;
         node;
         node = node->next)
    {
        Assert(index < array.count);
        array.strings[index++] = node->string;
    }
    return(array);
}

function i64
kv_fuzzy_search_forward(Application_Links *app, Buffer_ID buffer, i64 pos, String_Const_u8 needle, u64 *match_size)
{
    i64 buffer_size = buffer_get_size(app, buffer);
    i64 result = buffer_size;
    
    Scratch_Block temp(app);
    String_Const_u8_Array splits = kv_string_split_wildcards(temp, needle);
    if ( !splits.count ) { return result; }
    
    while( pos < buffer_size )
    {
        i64 original_pos = pos;
        String_Match first_match = buffer_seek_string(app, buffer, splits.strings[0], Scan_Forward, pos);
        if ( !first_match.buffer ) break;
        
        i64 match_start = first_match.range.min;
        i64 line_end    = get_line_end_pos_from_pos(app, buffer, match_start);
        pos = first_match.range.end - 1;
        b32 matched = true;
        for (i64 index = 1;
             index < splits.count;
             index++)
        {
            String_Const_u8 substring = splits.strings[index];
            String_Match match = buffer_seek_string(app, buffer, substring, Scan_Forward, pos);
            if ( match.buffer )
            {
                if ( match.range.max <= line_end )
                {
                    pos = match.range.end - 1;
                }
                else
                {
                    pos = get_line_start_pos_from_pos(app, buffer, match.range.start) - 1;
                    matched = false;
                    break;
                }
            }
            else
            {
                *match_size = first_match.range.max - first_match.range.min;
                return result;
            }
        }
        if ( matched )
        {
            *match_size = pos+1 - match_start;
            result = match_start;
            break;
        }
        
        if (!(pos > original_pos)) 
            return buffer_size;; 
    }
    
    return result;
}

function i64
kv_fuzzy_search_backward(Application_Links *app, Buffer_ID buffer, i64 pos, String_Const_u8 needle, u64 *match_size)
{
    i64 buffer_size = buffer_get_size(app, buffer); buffer_size;
    i64 result = -1;
    
    Scratch_Block temp(app);
    String_Const_u8_Array splits = kv_string_split_wildcards(temp, needle);
    if ( !splits.count ) { return result; }
    
    while( pos > -1 )
    {
        i64 original_pos = pos;
        String_Match first_match = buffer_seek_string(app, buffer, splits.strings[splits.count-1], Scan_Backward, pos);
        if( !first_match.buffer ) break;
        
        i64 match_start = first_match.range.max;
        i64 line_start   = get_line_start_pos_from_pos(app, buffer, match_start);
        pos = first_match.range.start;
        b32 matched = true;
        for (i64 index = splits.count-2;
             index >= 0;
             index--)
        {
            String_Const_u8 substring = splits.strings[index];
            String_Match match = buffer_seek_string(app, buffer, substring, Scan_Backward, pos);
            if ( match.buffer)
            {
                if ( match.range.min >= line_start )
                {
                    pos = match.range.start;
                }
                else
                {
                    pos = get_line_end_pos_from_pos(app, buffer, match.range.start);
                    matched = false;
                    break;
                }
            }
            else
            {
                // at the end of what it can find backwards
                return result;
            }
        }
        if ( matched )
        {
            *match_size = match_start - pos;
            result = pos;
            break;
        }
        
        if (!(pos < original_pos))
            return result;
    }
    
    return result;
}

#define is_alphanumeric(x) (('A' <= x && x <= 'Z') || ('a' <= x && x <= 'z') || ('0' <= x && x <= '9') || x == '_')
function u64 zk_move_alphaneumeric_boundry(String_Const_u8 string, u64 pos, Scan_Direction direction) {
    i64 i = pos;
    if (direction == Scan_Forward) {
        while (i < (i64)string.size && string.str[i] == ' ') i++;
        do {
            i++; 
        } while (i < (i64)string.size && is_alphanumeric(string.str[i]));
    }
    else if (direction == Scan_Backward) {
        while (i > 0 && string.str[i] == ' ') i--;
        do {
            i--;
        } while (i > 0 && is_alphanumeric(string.str[i]));
        }
    
    // clamp to valid range
    return (u64)clamp(0, i, (i64)string.size);
}

function String_Const_u8 
zk_buffer_get_string_under_cursor(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 pos, i64 *first_pos) {
    i64 buffer_size = buffer_get_size(app, buffer);
    
    String_Const_u8 result = {};
    if (!(0 <= pos && pos < buffer_size)) return result; 
    
    u8 *temp = push_array(arena, u8, 0x100);
    
    u8 backwards[256];
    // find characters before cursor
    u8 character = '_', j = 0;
    for (i64 i = pos; i > 0; i--) {
        buffer_read_range(app, buffer, Ii64(i, i - 1), &character);
        if (!is_alphanumeric(character)) break;
            backwards[j++] = character;
    }
    // write to main buffer 
    for (i64 i = 0; i < j; i++) {
        temp[i] = backwards[(j-1)-i];
    }
    
    Assert(first_pos);
    *first_pos = pos - j;
    
    // find characters after cursor
    character = '_';
    for (i64 i = pos; i < buffer_size && is_alphanumeric(character); i++) {
        buffer_read_range(app, buffer, Ii64(i, i + 1), &character);
        temp[j++] = character;  // TODO(ziv): range checks
    }
    temp[j] = '\0'; // TODO(ziv): check whether I Need this
    
    result.str = temp;
    result.size = j-1;
    return(result);
}
#undef is_alphanumeric


function void 
zk_delete_selection(Search_Bar *bar, Range_i64 selection) {
    
    /*             
                // b32 mod_sft = has_modifier(&in.event.key.modifiers, KeyCode_Shift);
                bar.string = (mod_ctl && !mod_sft ? qol_ctrl_backspace_string(app, bar.string) :
                              mod_ctl &&  mod_sft ? string_prefix(bar.string, 0) :
                              backspace_utf8(bar.string));
                 */
    
    // From: 
    // ccxxxccc
    //   ddd
    // 
    // To: 
    // cc
    //   ccc <- override original
    // 
    // Result: 
    // ccccc
    
    block_copy(bar->string.str+selection.min, 
               bar->string.str+selection.max, bar->string.size-selection.min);
    
    i64 final_size = bar->string.size - (selection.max - selection.min);
    bar->string.size = Max(0, final_size);
}

function void
zk_insert_string_with_selection(Search_Bar *bar, String_Const_u8 insert_str) {
    
    // TODO(ziv): bounds check
    
    // From
    // uuuuuulllllrrrrrr
    // To
    // uuuuuuiiirrrrrr
    // u- unchanged 
    // r- rhs (beginning of selection to delete), after which I need to insert new chars
    // l- lhs (beginning of characters to move to relocate)
    // i- inserted chars
    
    i64 lhs = bar->cursor_pos, rhs = bar->cursor_pos; 
    if (bar->is_selection_active) {
        Range_i64 range = Ii64(bar->cursor_pos, bar->anchor_pos);
        lhs = range.min; rhs = range.max;
        bar->is_selection_active = false;
    }
    
    i64 rhs_chars_count = (i64)bar->string.size - rhs;
    
    // copy rhs to new position
    char temp[256]; 
    block_copy(temp, bar->string.str + rhs, rhs_chars_count);
    block_copy(bar->string.str + lhs + insert_str.size,
               temp, rhs_chars_count);
    
    // copy inserted chars into new position
    block_copy(bar->string.str+lhs, 
               insert_str.str, insert_str.size);
    
    bar->string.size = lhs + insert_str.size + rhs_chars_count;
    bar->cursor_pos = lhs + insert_str.size;
}


function void
word_complete_iter_prev_wrapping(Word_Complete_Iterator *it){
    if (it->node == 0){
        it->node = it->list.last;
    }
    else if (it->node == it->list.first) {
        it->node = 0;
    }
    else{
        Node_String_Const_u8 *node = it->list.first;
        while (node != 0 && node->next != it->node){
            node = node->next;
        }
        it->node = node;
    }
}

function void 
zk_search_bar_word_complete(Application_Links *app, Buffer_ID buffer, 
                            Search_Bar *bar, b32 first_completion, b32 do_next)
{
    ProfileScope(app, "[ZK] search bar word complete");
    
    if (buffer != 0){
        Word_Complete_Iterator *it = word_complete_get_shared_iter(app);
        
        if (first_completion || !it->initialized){
            ProfileBlock(app, "[ZK] search bar word complete state init");
            
            String_Const_u8 needle = bar->string;
            needle.size = bar->cursor_pos; 
            
            it->initialized = false;
            if (needle.str != NULL && needle.size > 0) {
                word_complete_iter_init(buffer, needle, it);
            it->initialized = true;
            }
            }
        
        if (it->initialized){
            ProfileBlock(app, "[ZK] search bar word complete apply");
            
            if (do_next) 
                word_complete_iter_next(it);
            else 
                word_complete_iter_prev_wrapping(it);
            
            String_Const_u8 str = word_complete_iter_read(it);
            
            block_copy(bar->string.str, str.str, str.size);
            bar->string.size = str.size;

            it->range.max = it->range.min + str.size;
        }
    }
}

function void zk_view_set_highlights(Application_Links *app, Arena * arena, View_ID view, Range_i64 cursor_range,
                                         Range_i64 *all_matches_ranges, u64 all_matches_ranges_count) {
    
    // TODO(ziv): decide what should the lifetime of the search highlights
    // NOTE(ziv): for the time being I have decided on a only while search is still going on, you will have search highlights, I will use scratch memory. 
    
    // push_array(arena, Range_i64, ranges_count); 
    
    // view_highlight_range 
        
    // Set highlight for cursor position
    view_set_highlight_range(app, view, cursor_range);
    
    if (all_matches_ranges_count) {
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always); 
    // Set highlight for all matches 
    Managed_Scope scope = view_get_managed_scope(app, view);
    Managed_Object *all_matches = scope_attachment(app, scope, view_search_all_matches_highlights, Managed_Object);
    *all_matches = alloc_buffer_markers_on_buffer(app, buffer, (i32)all_matches_ranges_count*2, &scope);
    
    
    Temp_Memory temp = begin_temp(arena); 
    
    u64 marker_count = 2*all_matches_ranges_count; 
    Marker *markers = push_array(arena, Marker, marker_count); 
    for (int i = 0; i < all_matches_ranges_count; i++) {
        markers[2*i].pos   = all_matches_ranges[i].min;
        markers[2*i+1].pos = all_matches_ranges[i].max;
    }
        b32 success = managed_object_store_data(app, *all_matches, 0, (u32)marker_count, markers);
        success;
        end_temp(temp); 
        
        
        // test loading data
        Managed_Object *almtchs1 = scope_attachment(app, scope, view_search_all_matches_highlights, Managed_Object); 
        Marker mallmatchesrnge[6]; 
        if (managed_object_load_data(app, *almtchs1, 0, 3*2, mallmatchesrnge)) {
                          
            for (int i = 0; i < 1; i++) {
                Range_i64 range = Ii64(mallmatchesrnge[0].pos, mallmatchesrnge[1].pos);
            }
        }
        
    }
    // Set highlight for multi cursor selections
}


//- Main search function

function void
zk_isearch(Application_Links *app, Scan_Direction scan, i64 first_pos, String_Const_u8 query_init) {
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    if (!buffer_exists(app, buffer)){ return; }
    
    Search_Bar bar = {};
    if (!set_active_search_bar(app, view, &bar)) return; 
    
    Vec2_f32 old_margin = {};
    Vec2_f32 old_push_in = {};
    view_get_camera_bounds(app, view, &old_margin, &old_push_in);
    
    Vec2_f32 margin = V2f32(old_margin.x, clamp_bot(200.f, old_margin.y));
    view_set_camera_bounds(app, view, margin, old_push_in);
    
    u8 bar_string_space[256];
    bar.string = SCu8(bar_string_space, query_init.size);
    block_copy(bar.string.str, query_init.str, query_init.size);
    u64 match_size = bar.string.size;
    i64 pos = first_pos;
    
        // NOTE(ziv): Default behavior of selecting the inital query
    if (match_size != 0) {
        bar.anchor_pos = 0; 
        bar.cursor_pos = match_size;
        bar.is_selection_active = true;
    }
    
    Range_i64 range = buffer_range(app, buffer);
    
    // TODO(ziv): remove this!!!!
    //Range_i64 all_matches_ranges[10] = {}; 
    //u64 all_matches_ranges_count = 0;
    b32 is_last_action_autocomplete = false;
    
    Scratch_Block scratch(app); 
    
    User_Input in = {};
    for (;;){
        bar.prompt = (scan == Scan_Forward ?
                      string_u8_litexpr("I-Search: ") :
                      string_u8_litexpr("Reverse-I-Search: "));
         isearch__update_highlight(app, view, Ii64_size(pos, match_size));

/*         
        zk_view_set_highlights(app, scratch, view, 
                               Ii64_size(pos, match_size), 
                               all_matches_ranges,  
                               all_matches_ranges_count); 
         */

        in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
        if (in.abort){
            break;
        }
        
        String_Const_u8 string = to_writable(&in);
        
        // 
        // Special key bindings
        // 
        
         b32 string_change = false;
        if (match_key_code(&in, KeyCode_Return)) {
            u64 size = bar.string.size;
            size = clamp_top(size, sizeof(previous_isearch_query) - 1);
            block_copy(previous_isearch_query, bar.string.str, size);
            previous_isearch_query[size] = 0;
            break;
        }
        else if (match_key_code(&in, KeyCode_Tab)) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            zk_search_bar_word_complete(app, buffer, &bar, !is_last_action_autocomplete, !mod_ctl);
            bar.cursor_pos = bar.string.size; 
            string_change = true; is_last_action_autocomplete = true;
        }
        else if (match_key_code(&in, KeyCode_V)) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            if (mod_ctl) {
                String_Const_u8 clipboard_string = get_clipboard_index(&clipboard0, 0); 
                zk_insert_string_with_selection(&bar, clipboard_string); 
                string_change = true; is_last_action_autocomplete = false;
            }
        }
        else if (match_key_code(&in, KeyCode_C)) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            if (mod_ctl && bar.is_selection_active) {
                i32 clipboard_index = 0;  
                String_Const_u8 clipboard_string = string_substring(bar.string,  get_selection_range(bar));
                clipboard_post(clipboard_index, clipboard_string);
            }
        }
        
        //
        // Add/Delete characters & mark 'string_change'
        // 
        
        if (string.str != 0 && string.size > 0 && !string_change){
            zk_insert_string_with_selection(&bar, string); 
            string_change = true; 
        }
        else if (match_key_code(&in, KeyCode_Backspace)) { 
            // TODO(ziv): Update to make sure I handle utf8 also backspace_utf8(bar.string);
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            Range_i64 delete_range = (bar.is_selection_active ? get_selection_range(bar) : 
                               mod_ctl ? Ii64(zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Backward), bar.cursor_pos) :
                                      Ii64(clamp_bot(0, bar.cursor_pos-1), bar.cursor_pos));
            
            if (delete_range.min != delete_range.max) {
                zk_delete_selection(&bar, delete_range);
                bar.cursor_pos = delete_range.min;
                string_change = true; is_last_action_autocomplete = false;
            }
            
            bar.is_selection_active = false; 
        }
        else if (match_key_code(&in, KeyCode_Delete)) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            // TODO(ziv): Update to make sure I handle utf8 also backspace_utf8(bar.string);
            Range_i64 delete_range = (bar.is_selection_active ? get_selection_range(bar) :
                                      mod_ctl ? Ii64(zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Forward), bar.cursor_pos) :
                                      Ii64(clamp_top((i64)bar.string.size, bar.cursor_pos+1), bar.cursor_pos));
            
            if (delete_range.min != delete_range.max) {
                zk_delete_selection(&bar, delete_range);
                bar.cursor_pos = delete_range.min;
                string_change = true; is_last_action_autocomplete = false;
            }
            
            bar.is_selection_active = false; 
        }
        
        //
        // Movement & selection
        //
        
        if (in.event.kind == InputEventKind_KeyStroke && 
            (in.event.key.code == KeyCode_Right || 
            in.event.key.code == KeyCode_Left ||
            in.event.key.code == KeyCode_Home || 
             in.event.key.code == KeyCode_End)) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            b32 mod_sft = has_modifier(&in.event.key.modifiers, KeyCode_Shift);
            b32 is_special_case = false;
            
            if (mod_sft) {
                if (!bar.is_selection_active) {
                    bar.anchor_pos = bar.cursor_pos;
                }
                bar.is_selection_active = true;
            }
            else {
                
                // NOTE(ziv): special case - transition from selection, to no selection & direction
                if (bar.is_selection_active) {
                    is_special_case = true;
                    
                    // set cursor position depending on direction user wants (between anchor and cursor pos) 
                    if (in.event.key.code == KeyCode_Left) {
                        if (bar.cursor_pos > bar.anchor_pos) {
                            bar.cursor_pos = bar.anchor_pos;
                        }
                    }
                    else if (in.event.key.code == KeyCode_Right) {
                        if (bar.cursor_pos < bar.anchor_pos) {
                            bar.cursor_pos = bar.anchor_pos;
                        }
                    }
                }
                
                bar.is_selection_active = false;
            }
            
            if (!is_special_case) {
                                  
                  // Update cursor_pos depending on movement
                  switch (in.event.key.code) {
                    case KeyCode_Left:  { 
                        bar.cursor_pos = mod_ctl ? zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Backward) : Max(bar.cursor_pos-1, 0);
                      } break;
                      case KeyCode_Right: { 
                        bar.cursor_pos = mod_ctl ? zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Forward) : Min((i64)bar.string.size, bar.cursor_pos+1);
                    } break;
                case KeyCode_Home:  { bar.cursor_pos = 0; } break;
                case KeyCode_End:   { bar.cursor_pos = bar.string.size; } break;
                
                default: {
                } break;
            }
            }
            
            
        }
        
        // 
        // Do scan with/without 'string_change'
        // 
        
        b32 do_scan_action = false;
        b32 do_scroll_wheel = false;
        Scan_Direction change_scan = scan;
        if (!string_change){
            if (match_key_code(&in, KeyCode_Down) || match_key_code(&in, KeyCode_PageDown)){
                change_scan = Scan_Forward;
                do_scan_action = true;
            }
            else if (match_key_code(&in, KeyCode_Up) || match_key_code(&in, KeyCode_PageUp)){
                change_scan = Scan_Backward;
                do_scan_action = true;
            }
            else{
                leave_current_input_unhandled(app);
            }
        }
        
        if (string_change || do_scan_action){
            // search next if query has changed or user requested next/prev result
            scan = change_scan;
            i64 new_pos = 0;

            
            new_pos = (scan == Scan_Forward) ? 
                kv_fuzzy_search_forward(app, buffer, pos - string_change , bar.string, &match_size) : 
            kv_fuzzy_search_backward(app, buffer, pos + string_change, bar.string, &match_size);
            
            
            if (range_contains(range, new_pos)){
                pos = new_pos;
                // match_size = bar.string.size;
            }
        }
        else if (do_scroll_wheel){
            // if mouse movement.... scroll
            mouse_wheel_scroll(app);
        }
    }
    
    view_disable_highlight_range(app, view);
    
    if (in.abort){
        u64 size = bar.string.size;
        size = clamp_top(size, sizeof(previous_isearch_query) - 1);
        block_copy(previous_isearch_query, bar.string.str, size);
        previous_isearch_query[size] = 0;
        view_set_cursor_and_preferred_x(app, view, seek_pos(first_pos));
    }
    set_active_search_bar(app, view, NULL);
    
    view_set_camera_bounds(app, view, old_margin, old_push_in);
}

function void
qol_isearch(Application_Links *app, Scan_Direction scan, i64 first_pos, String_Const_u8 query_init){
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    if (!buffer_exists(app, buffer)){ return; }
    
    Query_Bar_Group group(app);
    Query_Bar bar = {};
    if (start_query_bar(app, &bar, 0) == 0){ return; }
    
    Vec2_f32 old_margin = {};
    Vec2_f32 old_push_in = {};
    view_get_camera_bounds(app, view, &old_margin, &old_push_in);
    
    Vec2_f32 margin = V2f32(old_margin.x, clamp_bot(200.f, old_margin.y));
    view_set_camera_bounds(app, view, margin, old_push_in);
    
    u8 bar_string_space[256];
    bar.string = SCu8(bar_string_space, query_init.size);
    block_copy(bar.string.str, query_init.str, query_init.size);
    u64 match_size = bar.string.size;
    i64 pos = first_pos;
    
    Range_i64 range = buffer_range(app, buffer);
    View_Context ctx = view_current_context(app, view);
    Command_Map *map = mapping_get_map(ctx.mapping, default_get_map_id(app, view));
    
    User_Input in = {};
    for (;;){
        bar.prompt = (scan == Scan_Forward ?
                      string_u8_litexpr("I-Search: ") :
                      string_u8_litexpr("Reverse-I-Search: "));
        isearch__update_highlight(app, view, Ii64_size(pos, match_size));
        
        in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
        if (in.abort){
            break;
        }
        
        String_Const_u8 string = to_writable(&in);
        
        b32 string_change = false;
        if (match_key_code(&in, KeyCode_Return) ||
            match_key_code(&in, KeyCode_Tab)){
            if (has_modifier(&in.event.key.modifiers, KeyCode_Control)){
                bar.string.size = cstring_length(previous_isearch_query);
                block_copy(bar.string.str, previous_isearch_query, bar.string.size);
            }
            else{
                u64 size = bar.string.size;
                size = clamp_top(size, sizeof(previous_isearch_query) - 1);
                block_copy(previous_isearch_query, bar.string.str, size);
                previous_isearch_query[size] = 0;
                break;
            }
        }
        else if (string.str != 0 && string.size > 0){
            String_u8 bar_string = Su8(bar.string, sizeof(bar_string_space));
            string_append(&bar_string, string);
            bar.string = bar_string.string;
            string_change = true;
        }
        else if (match_key_code(&in, KeyCode_Backspace)){
            String_Const_u8 old_string = bar.string;
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            b32 mod_sft = has_modifier(&in.event.key.modifiers, KeyCode_Shift);
            bar.string = (mod_ctl && !mod_sft ? qol_ctrl_backspace_string(app, bar.string) :
                          mod_ctl &&  mod_sft ? string_prefix(bar.string, 0) :
                          backspace_utf8(bar.string));
            string_change = (bar.string.size < old_string.size);
        }
        
        b32 do_scan_action = false;
        b32 do_scroll_wheel = false;
        Scan_Direction change_scan = scan;
        if (!string_change){
            if (match_key_code(&in, KeyCode_Down) || match_key_code(&in, KeyCode_PageDown)){
                change_scan = Scan_Forward;
                do_scan_action = true;
            }
            else if (match_key_code(&in, KeyCode_Up) || match_key_code(&in, KeyCode_PageUp)){
                change_scan = Scan_Backward;
                do_scan_action = true;
            }
            else{
                Custom_Command_Function *f = map_get_binding_recursive(ctx.mapping, map, &in.event);
                if (f == qol_search || f == qol_reverse_search){
                    change_scan = (f == qol_search ? Scan_Forward : Scan_Backward);
                    do_scan_action = true;
                }
                else if (f == qol_write_space || MCi_kind(f) == MC_Command_CursorPaste){
                    String_u8 bar_string = Su8(bar.string, sizeof(bar_string_space));
                    String_Const_u8 append = (f==qol_write_space ? string_u8_litexpr(" ") : get_clipboard_index(&clipboard0, 0));
                    string_append(&bar_string, append);
                    bar.string = bar_string.string;
                    string_change = true;
                }
                else if (f != 0){
                    Command_Metadata *metadata = get_command_metadata(f);
                    if (metadata != 0 && metadata->is_ui){
                        view_enqueue_command_function(app, view, f);
                        break;
                    }
                    if (MCi_kind(f) == MC_Command_Global){
                        f(app);
                    }
                }
                else {
                    leave_current_input_unhandled(app);
                }
            }
        }
        
        if (string_change || do_scan_action){
            scan = change_scan;
            i64 new_pos = 0;
            (scan == Scan_Forward ?
             seek_string_insensitive_forward(app, buffer, pos - string_change, 0, bar.string, &new_pos) :
             seek_string_insensitive_backward(app, buffer, pos + string_change, 0, bar.string, &new_pos));
            if (range_contains(range, new_pos)){
                pos = new_pos;
                match_size = bar.string.size;
            }
        }
        else if (do_scroll_wheel){
            mouse_wheel_scroll(app);
        }
    }
    
    view_disable_highlight_range(app, view);
    
    if (in.abort){
        u64 size = bar.string.size;
        size = clamp_top(size, sizeof(previous_isearch_query) - 1);
        block_copy(previous_isearch_query, bar.string.str, size);
        previous_isearch_query[size] = 0;
        view_set_cursor_and_preferred_x(app, view, seek_pos(first_pos));
    }
    
    view_set_camera_bounds(app, view, old_margin, old_push_in);
}

//- @Modification I have changed all qol functions into calling my own. 
//  TODO(ziv): Make sure to turn them all back and create my own function

function void
zk_isearch(Application_Links *app, Scan_Direction start_scan){
    Scratch_Block scratch(app);
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 cursor = view_get_cursor_pos(app, view);
    
    #if 0
    // get the token underneath the cursor
    Token_Array array = get_token_array_from_buffer(app, buffer);
    Token *token = get_token_from_pos(app, &array, cursor);
    String_Const_u8 query = push_token_lexeme(app, scratch, buffer, token);
    zk_isearch(app, start_scan, token->pos, query);
#endif 
    
    // get the string underneath the cursor
    i64 first_pos = cursor;
    String_Const_u8 query = zk_buffer_get_string_under_cursor(app, scratch, buffer, cursor, &first_pos); 
    zk_isearch(app, start_scan, first_pos, query);
}

//- Commands 

CUSTOM_COMMAND_SIG(qol_search)
CUSTOM_DOC("[QOL] I-search down")
{
    zk_isearch(app, Scan_Forward);
}

CUSTOM_COMMAND_SIG(qol_reverse_search)
CUSTOM_DOC("[QOL] I-search up")
{
    zk_isearch(app, Scan_Backward);
}
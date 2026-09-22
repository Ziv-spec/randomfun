// 4coder_better_search.cpp
// Ziv

//
// Searching on a word has the same behavior as vscode. It is similar to
// 'isearch_identifier' but, like vscode it selects the word such that if 
// you type anything, it deletes the selection and searches whatever you 
// type. Also selection with 'Shift' is supported across all commands that
// need it. 
//
// Also honestly, I think autocomplete here is useless. I wanted to play
// around with with so I added it, but I don't expect it to be useful to 
// myself or other people.
// 
// Fuzzy search down Down/PageDown
// Fuzzy search up   Up/PageUp
//
// Move              Left/Right
// Move boundry      Ctrl + Left/Right
// Move to end       End
// Move to start     Home
//
// Backspace word    Backspace
// Backspace boundry Ctrl + Backspace
// Delete word       Delete
// Delete boundry    Ctrl + Delete
//
// Copy Selection    Ctrl+C 
// Paste Selection   Ctrl+V
//
// Autocomplate      Tab
// R-Autocomplete    Ctrl+Tab
//
// Toggle Case Sensitive Alt + C
//
// Multi-Cursor Select/Unselect Down    Shift+Down
// Multi-Cursor Select/Unselect Up      Shift+Up
// Multi-Cursor Begin                   Return
//

// TODO(ziv): list_all_locations
// [x] Make new search buffer
// [x] make the function and handle jumping
// [x] handle multi-cursor
// [x] handle rendering highlights
// [ ] In search bar, for words that don't exits, color the extra characters in red
// [ ] Intergrate search & new search buffer format (*BIG* think about how to break it down)

#ifndef MC_Bind 
#error "This file uses the multi-cursor plugin by BYP in a non-optional manner.\nPlease use it in your custom layer first"
#endif

CUSTOM_ID(attachment, view_search_bar);
CUSTOM_ID(attachment, view_search_multi_cursor_highlights);
CUSTOM_ID(attachment, view_search_all_matches_highlights);

global String_Const_u8 search_results_name = string_u8_litexpr("*search-results*");
global float search_bar_blink = 0;

struct Search_Bar {
    String_Const_u8 prompt; 
    String_Const_u8 string; 
    
    b32 is_selection_active;
    i64 anchor_pos; // when selecting, this is the position selection
    // is anchord around when the cursor is moving (same as a marker)
    i64 cursor_pos; // cursor position in the search bar buffer utf8
    
    View_ID view;
};

//- Declarations 
// inner Hook functions, call from your own custom hooks
function void SEARCH_draw_highlights_inner(Application_Links *app, View_ID view, Text_Layout_ID text_layout_id);
function Rect_f32 SEARCH_draw_bar_inner(Application_Links *app, Rect_f32 region, View_ID view, Face_ID face_id);

// Functions that modify search buffer format. Look at *search-results*
internal void              zk_print_string_match_list_to_buffer(Application_Links *app, Buffer_ID out_buffer_id, String_Match_List matches);
internal Sticky_Jump_Array zk_parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_ID buffer);


// Helpers
function b32 set_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar);
function b32 get_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar);
inline function Range_i64 get_selection_range(Search_Bar bar);

function String_Const_u8_Array zk_string_split_wildcards(Arena *arena, String_Const_u8 string);

function void word_complete_iter_prev_wrapping(Word_Complete_Iterator *it);
function void zk_search_bar_word_complete(Application_Links *app, Buffer_ID buffer, 
                            Search_Bar *bar, b32 first_completion, b32 do_next);

function u64 zk_move_alphaneumeric_boundry(String_Const_u8 string, u64 pos, Scan_Direction direction); 
function i64 zk_cursor_move_one_backward_utf8(String_Const_u8 string, i64 pos);
function i64 zk_cursor_move_one_forward_utf8(String_Const_u8 string, i64 pos);

function void zk_delete_selection(Search_Bar *bar, Range_i64 selection);
function void zk_insert_string_with_selection(Search_Bar *bar, String_Const_u8 insert_str); 

function String_Const_u8 zk_buffer_get_string_under_cursor(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 pos, i64 *first_pos); 

//- Implementations


internal void
zk_print_string_match_list_to_buffer(Application_Links *app, Buffer_ID out_buffer_id, String_Match_List matches){
    Scratch_Block scratch(app);
    clear_buffer(app, out_buffer_id);
    Buffer_Insertion out = begin_buffer_insertion_at_buffered(app, out_buffer_id, 0, scratch, KB(64));
    buffer_set_setting(app, out_buffer_id, BufferSetting_ReadOnly, true);
    buffer_set_setting(app, out_buffer_id, BufferSetting_RecordsHistory, false);
    
    Temp_Memory buffer_name_restore_point = begin_temp(scratch);
    String_Const_u8 current_file_name = {};
    Buffer_ID current_buffer = 0;
    
    if (matches.first != 0){
        for (String_Match *node = matches.first;
             node != 0;
             node = node->next){
            if (node->buffer != out_buffer_id){
                if (current_buffer != 0 && current_buffer != node->buffer){
                    insertc(&out, '\n');
                }
                if (current_buffer != node->buffer){
                    end_temp(buffer_name_restore_point);
                    current_buffer = node->buffer;
                    current_file_name = push_buffer_file_name(app, scratch, current_buffer);
                    if (current_file_name.size == 0){
                        current_file_name = push_buffer_unique_name(app, scratch, current_buffer);
                    }
                    
                    insertf(&out, "%S:\n", current_file_name);
                }
                
                Buffer_Cursor cursor = buffer_compute_cursor(app, current_buffer, seek_pos(node->range.first));
                Temp_Memory line_temp = begin_temp(scratch);
                String_Const_u8 full_line_str = push_buffer_line(app, scratch, current_buffer, cursor.line);
                String_Const_u8 line_str = string_skip_chop_whitespace(full_line_str);
                insertf(&out, "%d,%d: %S\n", cursor.line, cursor.col, line_str);
                end_temp(line_temp);
            }
        }
    }
    else{
        insertf(&out, "no matches");
    }
    
    end_buffer_insertion(&out);
    lock_jump_buffer(app, out_buffer_id);
}

internal void
zk_print_all_matches_all_buffers(Application_Links *app, String_Const_u8_Array match_patterns, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, Buffer_ID out_buffer_id){
    Scratch_Block scratch(app);
    String_Match_List matches = find_all_matches_all_buffers(app, scratch, match_patterns, must_have_flags, must_not_have_flags);
    string_match_list_filter_remove_buffer(&matches, out_buffer_id);
    string_match_list_filter_remove_buffer_predicate(app, &matches, buffer_has_name_with_star);
    zk_print_string_match_list_to_buffer(app, out_buffer_id, matches);
}

internal void
zk_print_all_matches_all_buffers_to_search(Application_Links *app, String_Const_u8_Array match_patterns, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, View_ID default_target_view) {
    Buffer_ID search_buffer = create_or_switch_to_buffer_and_clear_by_name(app, search_results_name, default_target_view);
    zk_print_all_matches_all_buffers(app, match_patterns, must_have_flags, must_not_have_flags, search_buffer);
}

internal void
zk_list_all_locations__generic(Application_Links *app, String_Const_u8_Array needle, List_All_Locations_Flag flags){
    if (needle.count > 0){
        View_ID target_view = get_next_view_after_active(app, Access_Always);
        String_Match_Flag must_have_flags = 0;
        String_Match_Flag must_not_have_flags = 0;
        if (HasFlag(flags, ListAllLocationsFlag_CaseSensitive)){
            AddFlag(must_have_flags, StringMatch_CaseSensitive);
        }
        if (!HasFlag(flags, ListAllLocationsFlag_MatchSubstring)){
            AddFlag(must_not_have_flags, StringMatch_LeftSideSloppy);
            AddFlag(must_not_have_flags, StringMatch_RightSideSloppy);
        }
        zk_print_all_matches_all_buffers_to_search(app, needle, must_have_flags, must_not_have_flags, target_view);
    }
}

internal void
zk_list_all_locations__generic(Application_Links *app, String_Const_u8 needle, List_All_Locations_Flag flags){
    if (needle.size != 0){
        String_Const_u8_Array array = {&needle, 1};
        zk_list_all_locations__generic(app, array, flags);
    }
}

internal void
zk_list_all_locations__generic_query(Application_Links *app, List_All_Locations_Flag flags){
    Scratch_Block scratch(app);
    u8 *space = push_array(scratch, u8, KB(1));
    String_Const_u8 needle = get_query_string(app, "List Locations For: ", space, KB(1));
    zk_list_all_locations__generic(app, needle, flags);
}


CUSTOM_COMMAND_SIG(zk_list_all_locations)
CUSTOM_DOC("[zk] Queries the user for a string and lists all exact case-insensitive matches found in all open buffers.")
{
    zk_list_all_locations__generic_query(app, 0);
}


internal Sticky_Jump_Array
zk_parse_buffer_to_jump_array_inner(Application_Links *app, Arena *arena, Buffer_ID buffer){
    Sticky_Jump_Node *jump_first = 0;;
    Sticky_Jump_Node *jump_last = 0;
    i32 jump_count = 0;
    
    // *search-results* buffer format: 
    //
    // filename1:
    // line: *content*
    // line: *content*
    //
    // filename2:
    // line: *content*
    // ...
    
    Buffer_ID jump_buffer = {}; 
    for (i32 line = 1;; line += 1){
        b32 output_jump = false;
        i32 colon_index = 0;
        b32 is_sub_error = false;
        Buffer_ID out_buffer_id = 0;
        i64 out_pos = 0;
        
        {
            Temp_Memory_Block line_auto_closer(arena);
            if (is_valid_line(app, buffer, line)){
                String_Const_u8 line_str = push_buffer_line(app, arena, buffer, line);
                
                u64 colon_pos = string_find_first(line_str, 0, ':');
                String_Const_u8 line_col_or_filename = string_prefix(line_str, colon_pos);
                
                if (line_col_or_filename.size > 0 && ( '0' <= line_col_or_filename.str[0] && line_col_or_filename.str[0] <= '9' )) {
                    // parse line,col 
                    
                    u64 comma_pos = string_find_first(line_col_or_filename, 0, ',');
                    i64 bline = string_to_integer(string_prefix(line_col_or_filename, comma_pos), 10); 
                    i64 bcol  = string_to_integer(string_skip(line_col_or_filename, comma_pos+1), 10); 
                    
                    Buffer_Seek location = { buffer_seek_line_col };
                    location.line = bline;
                    location.col = bcol; 
                    
                    Buffer_Cursor cursor = buffer_compute_cursor(app, jump_buffer, location);
                    if (cursor.line > 0) {
                        out_buffer_id = jump_buffer;
                        out_pos = cursor.pos; 
                        output_jump = true; 
                    }
                }
                else if (line_col_or_filename.size > 0) {
                    // parse filename
                    
                    String_Const_u8 filename = string_prefix(line_str, line_str.size-1);
                    if (open_file(app, &jump_buffer, filename, false, true)){
                        if (buffer_exists(app, jump_buffer)){
                            out_buffer_id = jump_buffer;
                        }
                    }
                }
                
            }
            else{
                break;
            }
        }
        
        if (output_jump){
            Sticky_Jump_Node *jump = push_array(arena, Sticky_Jump_Node, 1);
            sll_queue_push(jump_first, jump_last, jump);
            jump_count += 1;
            jump->jump.list_line = line;
            jump->jump.list_colon_index = colon_index;
            jump->jump.is_sub_error =  is_sub_error;
            jump->jump.jump_buffer_id = out_buffer_id;
            jump->jump.jump_pos = out_pos;
        }
    }
    
    Sticky_Jump_Array result = {};
    result.count = jump_count;
    result.jumps = push_array(arena, Sticky_Jump, result.count);
    i32 index = 0;
    for (Sticky_Jump_Node *node = jump_first;
         node != 0;
         node = node->next){
        result.jumps[index] = node->jump;
        index += 1;
    }
    
    // here I should put the jump array into the highlighting attachement
    // first convert the array into ranges using the search bar query size
    // then go on... 
    
    return(result);
}

// NOTE(ziv): Must change 'init_marker_list' to use this function if you want the new jump 
// buffer to work well 
internal Sticky_Jump_Array
zk_parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_ID buffer){
    Buffer_ID search_buffer_id = get_buffer_by_name(app, string_u8_litexpr("*search*"), Access_Always); 
    if (search_buffer_id == buffer)
        return parse_buffer_to_jump_array(app, arena, buffer); 
    else 
        return zk_parse_buffer_to_jump_array_inner(app, arena, buffer);
    
}

//-


function b32 
set_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar) {
    Managed_Scope scope = view_get_managed_scope(app, view);
    Search_Bar **sb = scope_attachment(app, scope, view_search_bar, Search_Bar *);
    if (sb == NULL) 
        return false;
    *sb = bar;
    return true;
}

function b32 
get_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar) {
    Assert(bar != NULL); 
    Managed_Scope scope = view_get_managed_scope(app, view);
    Search_Bar **sb = scope_attachment(app, scope, view_search_bar, Search_Bar *);
    if (*sb == NULL) 
        return false;
    *bar = **sb;
    return true;
}

inline function Range_i64 get_selection_range(Search_Bar bar) {
    return Ii64(bar.cursor_pos, bar.anchor_pos);
}

function void 
SEARCH_draw_highlights_inner(Application_Links *app, View_ID view, Text_Layout_ID text_layout_id) {
    ProfileScope(app, "draw search highlights");
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    Managed_Scope scope = view_get_managed_scope(app, view);
        Range_i64_Array *all_matches = scope_attachment(app, scope, view_search_all_matches_highlights, Range_i64_Array);
    
    //local_persist int first_match_in_view = 0; 
    
    int first_match = 0; 

/*     
        int first_match = clamp_bot(first_match_in_view-1, 0);
    if (first_match_in_view < all_matches->count) {
        Range_i64 range = all_matches->ranges[first_match]; 
        if (range_contains(visible_range, range.min)) {
            first_match = 0;
        }
    }
     */

    f32 roundness = 0;
    for (int i = first_match; 
         i < all_matches->count; 
         ++i) {
        
        Range_i64 match = all_matches->ranges[i]; 
        if ((visible_range.min+1) < match.min ) { 
            if ( match.max < (visible_range.max-1)) {
            draw_character_block(app, text_layout_id, match, roundness, fcolor_id(defcolor_highlight_cursor_line));
                // paint_text_color(app, text_layout_id, match,  finalize_color(defcolor_at_highlight, 1));
            }
            else {
                break;
            }
            
        }
    }
    
    // Draw all multi-cursor highlights 
    if (all_matches->count > 0) {
    ARGB_Color back_color = fcolor_resolve(fcolor_id(defcolor_highlight));
    ARGB_Color text_color = fcolor_resolve(fcolor_id(defcolor_at_highlight));
    
    f32 alpha = .75f; 
    u8 alpha_char = (u8)(alpha * 255.f) << 24;
    back_color = back_color | alpha_char;
    text_color = text_color | alpha_char;
    
    for_mc(node, mc_context.cursors) {
            Range_i64 range = Ii64(node->cursor_pos, node->mark_pos);
            if ((visible_range.min+1) < range.min && range.max < (visible_range.max-1)) {
        draw_character_block(app, text_layout_id, range, 0, back_color);
                paint_text_color(app, text_layout_id, range, text_color);
            }
        }
        
    }
    
    }

function void 
SEARCH_tick_inner(Application_Links *app, Frame_Info frame_info) {
    search_bar_blink += frame_info.animation_dt; 
    search_bar_blink = (search_bar_blink < 4*3.141592653589793) ? search_bar_blink : 0; 
}

function Rect_f32 
SEARCH_draw_bar_inner(Application_Links *app, Rect_f32 region, View_ID view, Face_ID face_id) {
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
            
            f32 cursor_width = get_string_advance(app, face_id, SCu8(search_bar.string.str,search_bar.cursor_pos));
            f32 anchor_width = get_string_advance(app, face_id, SCu8(search_bar.string.str,search_bar.anchor_pos));
            
            cursor_x_pos = p.x + prompt_width + cursor_width;
            anchor_x_pos = p.x + prompt_width + anchor_width;
        }
        
        // Draw selection
        if (search_bar.is_selection_active) {
            Range_f32 sel = If32(anchor_x_pos, cursor_x_pos);
            Rect_f32 rect = Rf32_xy_wh(sel.min, p.y, sel.max - sel.min, face_metrics.line_height); 
            draw_rectangle_fcolor(app, rect, 0.f, fcolor_id(defcolor_at_highlight, 0));
        }
        
        // Draw text
        p = draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
        
        
        b32 is_active_view = search_bar.view == get_active_view(app, Access_Always);
        if (is_active_view) {
        if(search_bar_blink < 10.f) {
            animate_in_n_milliseconds(app, 0); 
        }
        
#define ACTIVE_BLINK(b) (sin_f32(7.f*b) >= 0.f || (b) > 5.f)
        if (ACTIVE_BLINK(search_bar_blink)) {
            // Draw cursor rect
            draw_rectangle_fcolor(app, Rf32_xy_wh(cursor_x_pos, p.y, 2.f, face_metrics.line_height), 0.f, fcolor_id(defcolor_cursor, 0));
        }
        }
        
        region = pair.min;
    }
    
    return(region);
}

//- Helpers 

// Fuzzy search
function String_Const_u8_Array
zk_string_split_wildcards(Arena *arena, String_Const_u8 string)
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

function String_Match_List
zk_fuzzy_find_matches_forward_in_range(Application_Links *app, Buffer_ID buffer, Arena *arena, String_Const_u8 needle, String_Const_u8_Array splits, i64 begin, i64 end, i64 *match_count) {
    String_Match_List list = {}; 
    
    i64 pos = begin;
    while (pos < end) {
        
        String_Match first_match = buffer_seek_string(app, buffer, splits.strings[0], Scan_Forward, pos);
        if ( !first_match.buffer ) break;
        if ( end < first_match.range.min ) break; 
        
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
                String_Match *match_node = push_array(arena, String_Match, 1);
                match_node->range = first_match.range; 
                sll_queue_push(list.first, list.last, match_node); 
                match_count[0]++;
                matched = false;
                break;
            }
        }
        if ( matched )
        {
            String_Match *match_node= push_array(arena, String_Match, 1);
            match_node->range = Ii64(match_start, pos+1); 
            sll_queue_push(list.first, list.last, match_node); 
            match_count[0]++;
        }
    }
    
    
    return list;
}

function Range_i64_Array
zk_fuzzy_find_matches_buffer(Application_Links *app, Arena *arena, Buffer_ID buffer, String_Const_u8 needle, i64 pos, i64 *forward_match_idx) {
    ProfileScope(app, "[ZK] fuzzy find all matches in buffer");
    
    String_Const_u8_Array splits = zk_string_split_wildcards(arena, needle);
    if ( !splits.count ) { return Range_i64_Array{0}; }
    
    i64 buffer_size = buffer_get_size(app, buffer); 
    
    i64 match_count_pre = 0,  match_count_post = 0;
    String_Match_List matches_pre  = zk_fuzzy_find_matches_forward_in_range(app, buffer, arena, needle, splits, 0, pos, &match_count_pre);
    String_Match_List matches_post = zk_fuzzy_find_matches_forward_in_range(app, buffer, arena, needle, splits, pos, buffer_size, &match_count_post);
    
    int i = 0; 
    Range_i64 *ranges_array = push_array(arena, Range_i64, match_count_pre+match_count_post); 
    for (String_Match *node = matches_pre.first; node != 0; node = node->next) {
        ranges_array[i++] = node->range;
    }
    for (String_Match *node = matches_post.first; node != 0; node = node->next) {
        ranges_array[i++] = node->range;
    }
    
    Range_i64_Array result = {
        ranges_array,
        (i32)(match_count_pre+match_count_post)
    };
    
    Assert(forward_match_idx); 
    *forward_match_idx = match_count_pre; 

    return result;
}

// Autocomplete
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

// Movement
function u64 
zk_move_alphaneumeric_boundry(String_Const_u8 string, u64 pos, Scan_Direction direction) {
    i64 i = pos;
    if (direction == Scan_Forward) {
        while (i < (i64)string.size && string.str[i] == ' ') i++;
        do {
            i++; 
        } while (i < (i64)string.size && character_is_alpha_numeric_unicode(string.str[i]));
    }
    else if (direction == Scan_Backward) {
        while (i > 0 && string.str[i] == ' ') i--;
        do {
            i--;
        } while (i > 0 && character_is_alpha_numeric_unicode(string.str[i]));
        }
    
    return (u64)clamp(0, i, (i64)string.size);
}

function i64
zk_cursor_move_one_backward_utf8(String_Const_u8 string, i64 pos){
    if (string.size > 0){
        i64 i = pos-1;
        for (; i > 0; --i){
            if (string.str[i] <= 0x7F || string.str[i] >= 0xC0){
                break;
            }
        }
        return Max(i, 0);
    }
    return(0);
}

function i64
zk_cursor_move_one_forward_utf8(String_Const_u8 string, i64 pos){
    if (string.size > 0){
        u64 i = pos+1;
        for (; i < string.size; ++i){
            if (string.str[i] <= 0x7F || string.str[i] >= 0xC0){
                break;
            }
        }
        return Min(i, string.size);
    }
    return(0);
}

// Selection delete/insert
function void 
zk_delete_selection(Search_Bar *bar, Range_i64 selection) {
    Assert(bar != NULL); 
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

// NOTE: didn't want all this code below so I put it here :)
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
        if (!character_is_alpha_numeric_unicode(character)) break;
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
    for (i64 i = pos; i < buffer_size && character_is_alpha_numeric_unicode(character); i++) {
        buffer_read_range(app, buffer, Ii64(i, i + 1), &character);
        temp[j++] = character;  // TODO(ziv): range checks
    }
    temp[j] = '\0'; // TODO(ziv): check whether I Need this
    
    result.str = temp;
    result.size = j-1;
    return(result);
}

function void
zk_view_disable_highlight_range(Application_Links *app, View_ID view) {
    view_disable_highlight_range(app, view);
    
    Managed_Scope scope = view_get_managed_scope(app, view);
    Range_i64_Array *all_matches = scope_attachment(app, scope, view_search_all_matches_highlights, Range_i64_Array);
    all_matches->count = 0; 
}

function void
zk_view_set_highlight_range(Application_Links *app, View_ID view, Range_i64_Array matches_to_highlight) {
    
    Managed_Scope scope = view_get_managed_scope(app, view);
    Range_i64_Array *all_matches_to_highlight = scope_attachment(app, scope, view_search_all_matches_highlights, Range_i64_Array);
    *all_matches_to_highlight = matches_to_highlight;
    
    
}


//- Main search function

function void
zk_isearch(Application_Links *app, Scan_Direction scan, i64 first_pos, String_Const_u8 query_init) {
    Scratch_Block scratch(app); 
    Temp_Memory temp = begin_temp(scratch);
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    if (!buffer_exists(app, buffer)){ return; }
    
    Search_Bar bar = {};
    bar.view = view; 
    if (!set_active_search_bar(app, view, &bar)) return; 
    
    Vec2_f32 old_margin = {};
    Vec2_f32 old_push_in = {};
    view_get_camera_bounds(app, view, &old_margin, &old_push_in);
    
    Vec2_f32 margin = V2f32(old_margin.x, clamp_bot(200.f, old_margin.y));
    view_set_camera_bounds(app, view, margin, old_push_in);
    
    u8 bar_string_space[256];
    bar.string = SCu8(bar_string_space, query_init.size);
    block_copy(bar.string.str, query_init.str, query_init.size);
    
    i64 match_idx = 0;
    Range_i64 match_range = { first_pos, first_pos+(i64)bar.string.size }; 
    Range_i64_Array all_matches = zk_fuzzy_find_matches_buffer(app, scratch, buffer, bar.string, match_range.min-1, &match_idx);
    zk_view_set_highlight_range(app, view, all_matches); 
    
        bar.anchor_pos = 0; 
        bar.cursor_pos = bar.string.size;
        bar.is_selection_active = true;
    
    Range_i64 range = buffer_range(app, buffer);
    b32 is_last_action_autocomplete = false;
    
    MC_end(app); 
    
    
    User_Input in = {};
    for (;;){
        bar.prompt = (scan == Scan_Forward ?
                      string_u8_litexpr("I-Search: ") :
                      string_u8_litexpr("Reverse-I-Search: "));
         isearch__update_highlight(app, view, match_range);
        zk_view_set_highlight_range(app, view, all_matches); 
        
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
            
            // I do this to avoid having MC cursor and the actual cursor
            // on the same spot, casuing doubling of input on one spot
            {
            i64 current_cursor_pos = match_range.max;
            MC_remove(app, current_cursor_pos);
            view_set_cursor(app, view, seek_pos(current_cursor_pos));
                
                MC_begin(app);
            }
            
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
                string_change = true;
            }
        }
        
        //
        // Add/Delete characters & mark 'string_change'
        // 
        
        Range_i64 delete_range;
        if (string.str != 0 && string.size > 0 && !string_change){
            zk_insert_string_with_selection(&bar, string); 
            string_change = true; 
        }
        else if (match_key_code(&in, KeyCode_Backspace)) { 
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            delete_range = (bar.is_selection_active ? get_selection_range(bar) : 
                               mod_ctl ? Ii64(zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Backward), bar.cursor_pos) :
                                      Ii64(zk_cursor_move_one_backward_utf8(bar.string, bar.cursor_pos), bar.cursor_pos));
            
            DELETE_THE_RANGE:
            
            if (delete_range.min != delete_range.max) {
                zk_delete_selection(&bar, delete_range);
                bar.cursor_pos = delete_range.min;
                
                string_change = true; 
                is_last_action_autocomplete = false;
            }
            bar.is_selection_active = false; 
            
        }
        else if (match_key_code(&in, KeyCode_Delete)) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            delete_range = (bar.is_selection_active ? get_selection_range(bar) :
                                      mod_ctl ? Ii64(zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Forward), bar.cursor_pos) :
                                      Ii64(zk_cursor_move_one_forward_utf8(bar.string, bar.cursor_pos), bar.cursor_pos));
            
            goto DELETE_THE_RANGE;
        }
        
        //
        // Movement & selection
        //
        
        if (in.event.kind == InputEventKind_KeyStroke && 
            (in.event.key.code == KeyCode_Right || in.event.key.code == KeyCode_Left ||
            in.event.key.code == KeyCode_Home || in.event.key.code == KeyCode_End)) {
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
                                  
                  switch (in.event.key.code) {
                    case KeyCode_Left:  { 
                        bar.cursor_pos = mod_ctl ? zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Backward) : zk_cursor_move_one_backward_utf8(bar.string, bar.cursor_pos);
                        
                      } break;
                      case KeyCode_Right: { 
                        bar.cursor_pos = mod_ctl ? zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Forward) : zk_cursor_move_one_forward_utf8(bar.string, bar.cursor_pos);
                    } break;
                case KeyCode_Home:  { bar.cursor_pos = 0; } break;
                case KeyCode_End:   { bar.cursor_pos = bar.string.size; } break;
                
                default: {
                } break;
            }
            }
            
            
        }
        
        // 
        // Do scan
        // 
        
        b32 do_scan_action = false;
        b32 do_scroll_wheel = false;
        if (!string_change){
            if (match_key_code(&in, KeyCode_Down) || match_key_code(&in, KeyCode_PageDown)){
                scan = Scan_Forward; 
                do_scan_action = true; 
            }
            else if (match_key_code(&in, KeyCode_Up) || match_key_code(&in, KeyCode_PageUp)){
                scan = Scan_Backward; 
                do_scan_action = true;
            }
            else{
                leave_current_input_unhandled(app);
            }
        }
        
        //
        // Handle string change
        //
        
        if (string_change || do_scan_action) {
            search_bar_blink = 0;
            
            if (string_change) {
                MC_end(app);
                end_temp(temp); // NOTE(ziv): trick for 'zk_fuzzy_find_matches_buffer' 
                // to clear memory right before the function allocates 
                
                all_matches = zk_fuzzy_find_matches_buffer(app, scratch, buffer, bar.string, match_range.min-1, &match_idx);
                
                if (all_matches.count ) {
                    if (0 <= match_idx && match_idx <= all_matches.count) {
                match_range = all_matches.ranges[match_idx]; 
                }
                }
                
            }
            else if (all_matches.count) {
                
                b32 mode_sft = has_modifier(&in.event.key.modifiers, KeyCode_Shift);
                if (mode_sft) {
                    b32 duplicate = false; 
                    for_mc(node, mc_context.cursors) {
                        if (node->cursor_pos == match_range.max) {
                            duplicate = true; 
                            break;
                        }
                    }
                    
                    if (duplicate) {
                        MC_remove(app, match_range.max);
                    }
                    else { 
                        MC_insert(app, match_range.max, match_range.min);
                    }
                }
                
                match_idx = (scan == Scan_Forward) ? 
                    clamp_top(match_idx+1, all_matches.count-1) : 
                match_idx = clamp_bot(match_idx-1, 0);
                
                      match_range = all_matches.ranges[match_idx]; 
            }
            
        }
        else if (do_scroll_wheel){
            // if mouse movement.... scroll
            mouse_wheel_scroll(app);
        }
    }
    
    zk_view_disable_highlight_range(app, view);
    
    if (in.abort){
        MC_end(app); 
        
        u64 size = bar.string.size;
        size = clamp_top(size, sizeof(previous_isearch_query) - 1);
        block_copy(previous_isearch_query, bar.string.str, size);
        previous_isearch_query[size] = 0;
        view_set_cursor_and_preferred_x(app, view, seek_pos(first_pos));
    }
    set_active_search_bar(app, view, NULL);
    
    view_set_camera_bounds(app, view, old_margin, old_push_in);
}

//- 

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

function void
zk_isearch(Application_Links *app, Scan_Direction start_scan){
    Scratch_Block scratch(app);
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 cursor = view_get_cursor_pos(app, view);
    
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
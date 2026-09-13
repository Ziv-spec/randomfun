
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
// Delete & control
// fix shift 
// restructure code
// make sure delete & adding a character considers the selection and everything 
// add autocomplete
// 
// add highlighting for search results
// multi cursor support 
// have support for search in all files and so on.. look into #showcase in discord
// 





//
CUSTOM_ID(attachment, view_search_bar);

struct Search_Bar {
    String_Const_u8 prompt; 
    String_Const_u8 string; 
    
    b32 selection_draw_on;
    i64 selection_anchor; // from here the selection begins
    i64 cursor_pos;       // cursor position in the search bar
    
    u64 string_capacity; // TODO(ziv): figure out why is this needed
};




//- Declarations 
// Used to render improved query bar functionality, and highlight all matches
// function Rect_f32 SEARCH_render_query_bar(Application_Links *app, Rect_f32 region, View_ID view, Face_ID face_id);
function void SEARCH_render_search_highlights(View_ID view);

// 


//- Implementations

// Management of search bar state

function void SEARCH_init(Application_Links *app, View_ID view) {
    //Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Managed_Scope scope = view_get_managed_scope(app, view);
    Managed_Object *search_bar = scope_attachment(app, scope, view_search_bar, Managed_Object);
    
    // Initialize memory for search_bar
    Managed_Object search_bar_obj = alloc_managed_memory_in_scope(app, scope, sizeof(Search_Bar), 1);
    
    // Set default data to be empty
    Search_Bar sbar = {
        string_u8_litexpr(""),
        string_u8_litexpr(""),
    };
    managed_object_store_data(app, search_bar_obj, 0, 1, &sbar);
    
    // Attache allocated search bar object to scope_attachement
    *search_bar = search_bar_obj;
    
}

function b32 set_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar) {
    Managed_Scope scope = view_get_managed_scope(app, view);
    Search_Bar **sb = scope_attachment(app, scope, view_search_bar, Search_Bar *);
    // Set search bar data
    *sb = bar;
    return true;
}

function b32 get_active_search_bar(Application_Links *app, View_ID view, Search_Bar *bar) {
    Managed_Scope scope = view_get_managed_scope(app, view);
    Search_Bar **sb = scope_attachment(app, scope, view_search_bar, Search_Bar *);
    if (*sb == NULL) {
        return false;
    }
        *bar = **sb;
    return true;
}

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
                (u64)search_bar.selection_anchor
            };
            
            Fancy_Line achr_list = {};
            push_fancy_string(scratch, &achr_list, fcolor_id(defcolor_pop1),
                              string_up_to_anchor);
            anchor_x_pos = p.x + prompt_width + get_fancy_line_width(app, face_id, &achr_list); 
        }
        
        
        // Draw selection
        if (search_bar.selection_draw_on) {
            
            f32 width = 0; 
            f32 begin_pos = 0;
            if (cursor_x_pos < anchor_x_pos) {
                width = anchor_x_pos-cursor_x_pos;
                begin_pos = cursor_x_pos; 
            }
            else  {
                width = cursor_x_pos-anchor_x_pos;
                begin_pos = anchor_x_pos; 
            }
            
            draw_rectangle_fcolor(app, Rf32_xy_wh(begin_pos, p.y, width, face_metrics.line_height), 0.f, fcolor_id(defcolor_at_highlight, 0));
            
        }
        
        // Draw text
        p = draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
        
        // Draw cursor rect
        draw_rectangle_fcolor(app, Rf32_xy_wh(cursor_x_pos, p.y, 2.f, face_metrics.line_height), 0.f, fcolor_id(defcolor_cursor, 0));

        region = pair.min;
    }
    
return(region);
}

//- Commands 



function u64 zk_move_alphaneumeric_boundry(String_Const_u8 string, u64 pos, Scan_Direction direction) {
    
#define is_alphanumeric(x) (('A' <= x && x <= 'Z') || ('a' <= x && x <= 'z') || ('0' <= x && x <= '9') || x == '_')
    i64 i = pos;
    if (direction == Scan_Forward) {
        do {
            i++;
        } while (i < (i64)string.size && is_alphanumeric(string.str[i]));
    }
    else if (direction == Scan_Backward) {
        do {
            i--;
        } while (i > 0 && is_alphanumeric(string.str[i]));
        }
    #undef is_alphanumeric
    return (u64)clamp(0, i, (i64)string.size);
}



function b32 zk_do_selection(Search_Bar *bar, b32 mod_sft) {
    if (mod_sft) {
        if (!bar->selection_draw_on) {
            // set a new anchor only at beginning of selection
            bar->selection_anchor = bar->cursor_pos;
        }
        bar->selection_draw_on = true;
    }
    else if (bar->selection_draw_on) {
        // I return true when we change from selection mode, to normal mode
        bar->selection_draw_on = false;
        return true;
    }
    else {
        bar->selection_draw_on = false;
    }
    
    return false;
}

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
    
    Range_i64 range = buffer_range(app, buffer);
    
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
        
        if (match_key_code(&in, KeyCode_Return) ||
            match_key_code(&in, KeyCode_Tab)) {
        }
        
        //
        // Add/Delete characters & mark 'string_change'
        // 
        
        b32 string_change = false;
        if (string.str != 0 && string.size > 0){
            // append character and mark 'string_change' to true
            
            // TODO(ziv): use rhs, lhs terminology (there can be no middle
            // because the true lhs always stays intact, so I can use lhs, rhs).
            
            i64 cursor_pos_after_delete_selection = bar.cursor_pos;
            i64 characters_after_selection = bar.cursor_pos; 
            if (bar.selection_draw_on) {
                // override selection
                
                Range_i64 delete_range = Ii64(bar.cursor_pos, bar.selection_anchor); 
                //i64 delete_amount = delete_range.end - delete_range.start;
                
                cursor_pos_after_delete_selection = delete_range.min;
                characters_after_selection = delete_range.max;
                
            }
            
            
            
            
            // Insert into new position
            // TODO(ziv): add capacity checks to make sure to not overflow
            // copy old into new pos using intermediate buffer
            char temp[256]; 
            block_copy(temp,
                       bar.string.str+characters_after_selection,
                       (i64)bar.string.size - characters_after_selection);
            
            block_copy(bar.string.str+cursor_pos_after_delete_selection+string.size, 
                       temp, 
                       (i64)bar.string.size - characters_after_selection);
            
            // copy new into pos
            block_copy(bar.string.str+cursor_pos_after_delete_selection, 
                       string.str, 
                       string.size); 
            bar.string.size += string.size;

            
            bar.cursor_pos += string.size;
            
            string_change = true;
        }
        else if (match_key_code(&in, KeyCode_Backspace)){
            // handle deleting chars
            String_Const_u8 old_string = bar.string;
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            

/*             
            // b32 mod_sft = has_modifier(&in.event.key.modifiers, KeyCode_Shift);
            bar.string = (mod_ctl && !mod_sft ? qol_ctrl_backspace_string(app, bar.string) :
                          mod_ctl &&  mod_sft ? string_prefix(bar.string, 0) :
                          backspace_utf8(bar.string));
             */
            
            Range_i64 delete_range = (bar.selection_draw_on ? Ii64(bar.cursor_pos, bar.selection_anchor) : 
                               mod_ctl ? Ii64(zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Backward), bar.cursor_pos) :
                                      Ii64(clamp_bot(0, bar.cursor_pos-1), bar.cursor_pos));
            // TODO(ziv): Update to make sure I handle utf8 also backspace_utf8(bar.string);
            
            if (delete_range.min != delete_range.max) {
            i64 delete_size = (delete_range.max - delete_range.min);
                bar.string.size = Max(0, bar.string.size - delete_size);
            
            block_copy(bar.string.str+delete_range.min, 
                       bar.string.str+delete_range.max, 
                       bar.string.size-delete_range.min);
                
            bar.cursor_pos = delete_range.min;
            }
            string_change = (bar.string.size < old_string.size);
            
        }
        
        if (string_change) {
             // update search bar cursor position
        }
        
        //
        // Movement & selection
        //
        
        if (in.event.kind == InputEventKind_KeyStroke) {
            b32 mod_ctl = has_modifier(&in.event.key.modifiers, KeyCode_Control);
            b32 mod_sft = has_modifier(&in.event.key.modifiers, KeyCode_Shift);
            b32 changed = zk_do_selection(&bar, mod_sft); 
            
            // TODO(ziv): make sure shift+- to create underscore doesn't highlight by mistake
            
            if (!changed) { // constantly selecting or constatly not selecting
                
                // Update cursor_pos depending on movement
                switch (in.event.key.code) {
                    case KeyCode_Left:  { 
                        // NOTE(ziv): I make sure unsigned position does not overflow
                        // possibly you could just use unsigned values and make this more 
                        // readable
                        if (mod_ctl) {
                            // move token boundery
                            bar.cursor_pos = zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Backward);
                        }
                        else {
                            bar.cursor_pos = Max(bar.cursor_pos-1, 0); 
                        }
                    } break;
                    case KeyCode_Right: { 
                        if (mod_ctl) {
                            bar.cursor_pos = zk_move_alphaneumeric_boundry(bar.string, bar.cursor_pos, Scan_Forward);
                        }
                        else {
                            bar.cursor_pos = Min((i64)bar.string.size, bar.cursor_pos+1); 
                        }
                        
                    } break;
                case KeyCode_Home:  { bar.cursor_pos = 0; } break;
                case KeyCode_End:   { bar.cursor_pos = bar.string.size; } break;
                
                default: {
                } break;
            }
            }
            else { // changed from selecting to not selecting
                
                // set cursor position depending on direction user wants (between anchor and cursor pos) 
                if (in.event.key.code == KeyCode_Left) {
                    if (bar.cursor_pos > bar.selection_anchor) {
                        bar.cursor_pos = bar.selection_anchor;
                    }
                }
                else if (in.event.key.code == KeyCode_Right) {
                    if (bar.cursor_pos < bar.selection_anchor) {
                        bar.cursor_pos = bar.selection_anchor;
                    }
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
            (scan == Scan_Forward ?
             seek_string_insensitive_forward(app, buffer, pos - string_change, 0, bar.string, &new_pos) :
             seek_string_insensitive_backward(app, buffer, pos + string_change, 0, bar.string, &new_pos));
            if (range_contains(range, new_pos)){
                pos = new_pos;
                match_size = bar.string.size;
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
    View_ID view = get_active_view(app, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    zk_isearch(app, start_scan, pos, string_u8_empty);
}

CUSTOM_COMMAND_SIG(qol_search_identifier)
CUSTOM_DOC("[QOL] I-search identifier under cursor")
{
      Scratch_Block scratch(app);
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    Token_Array array = get_token_array_from_buffer(app, buffer);
    i64 cursor = view_get_cursor_pos(app, view);
    Token *token = get_token_from_pos(app, &array, cursor);
    String_Const_u8 query = push_token_lexeme(app, scratch, buffer, token);
    zk_isearch(app, Scan_Forward, token->pos, query);
}

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
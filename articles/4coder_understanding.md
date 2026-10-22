
This document aims at giving the general information helpful for writing a custom layer for 4coder.

# Basics

4coder is built for customization using ordinary C/C++ code. It exposes 3 ways of customizing your layer to make it feel like your own editor. 

1. Commands
2. Bindings
3. Hooks

+--------------+         +----------+
|     Core     |         |  Custom  |
+--------------+         |  Layer   |
| Input        | -- \    +----------+
| Layout       | --  \   | Commands |
| Drawing      | --   \  | Bindings |
| Views        | --    \ |          |
| Buffers      | --    / | anything |
| Panels       | --   /  |  else    |
| Memory       | --  /   |    .     |
| OS Funcs     | -- /    |    .     |
|              |         |          |
| Hook slots   | <------ | Hooks    |
+--------------+         +----------+

## Commands

Commands are nothing more than functions with the editor state that can do some action. e.g. insert text, delete a character, find a substring and so on. In 4coder you can search for commands to execute them or create shortcuts to do the same. To see the list of all commands, inside 4coder press Alt+X you will be placed into the command lister. There, you will be able to see all commands the custom layer provides for you to execute. 

If you want to create new commands just write functions in the following manner:

```c
CUSTOM_COMMAND_SIG(some_command)
CUSTOM_DOC("Here is the doc for this specific command.")
{
	// ...
}
```

4coder uses a meta program to scan for `CUSTOM_COMMAND_SIG` and `CUSTOM_DOC` and creates internal tables other parts of the program later uses. This means that aside from declaring your functions like that, you are required NO MORE WORK to create a command. When later opening the command lister, will see your new command automatically there along with whatever binding you created.

As for the documentation of the command, it will automatically appear in the command documentation lister if you first Alt+X for command lister then search for "command_documentation" and press enter. 

## Binding 

Of course having a command pop up in the command lister is nice, and searching for it is also cool, but to be productive, people usualy want to access these commands through keyboard shortcuts. This is where 4coder provides both a dynamic way to map keyboard input into commands and a default static one.

The common way you are expected to change your bindings is through the 'bindings.4coder' file provided with 4coder. The file defines names that must have 1 to 1 correspondence to the names insdie the custom layer. 

4coder provides default names to different scopes commands can be activated in. 

keys_global - bindings are accessible from everywhere
keys_file   - bindings are accessible inside a file or general buffer
keys_code   - bindings are specific to files that 4coder treats as code

bindings.4coder
```
keys_global = {
    { "exit_4coder", "F4", "Alt" }, // { command, key, modifier }
};

keys_file = {
	// ... 
};
```

Inside of the custom layer the names stay the same but the API for adding bindings is slightly different. 

'4coder_default_map.cpp'
```c
MappingScope();
SelectMapping(mapping);
    
String_ID global_id = vars_save_string_lit("keys_global"); // same name as in bindings
SelectMap(global_id);
Bind(exit_4coder, KeyCode_F4, KeyCode_Alt);
// ... 
String_ID file_id = vars_save_string_lit("keys_file");
SelectMap(file_id);
// ...
```

As you can see both use the same names to reference the scope they talk about, and the same name for commands that exist in 4coder. This is an important detail since interally what happens is the populating of a shared table where names and keys are resolved. Then when a user hits keys, a function called 'implicit_map_function' figures out what scope you are at e.g. 'keys_global' looks at the table we created earlier, and returns the appropreate command to execute. 

This means that when you use the custom layer you have access to the entire system if you want to have more complex input handling you can. Any type of input handilng is possible if you are willing to put the time and code it up.

# Hooks

Hooks are the only part in 4code where the custom layer needs to provide something to the core. They are set at initialization and thereafter called by the core at an appropriate time:

```c
// In your custom_layer_init()
set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);

set_custom_hook(app, HookID_ViewEventHandler,  default_view_input_handler);
set_custom_hook(app, HookID_Tick, default_tick);
set_custom_hook(app, HookID_RenderCaller, default_render_caller);
set_custom_hook(app, HookID_WholeScreenRenderCaller, default_whole_screen_render_caller);
set_custom_hook(app, HookID_DeltaRule, fixed_time_cubic_delta);
set_custom_hook_memory_size(app, HookID_DeltaRule,
                            delta_ctx_size(fixed_time_cubic_delta_memory_size));
set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
set_custom_hook(app, HookID_BeginBuffer, default_begin_buffer);
set_custom_hook(app, HookID_EndBuffer, end_buffer_close_jump_list);
set_custom_hook(app, HookID_NewFile, default_new_file);
set_custom_hook(app, HookID_SaveFile, default_file_save);
set_custom_hook(app, HookID_BufferEditRange, default_buffer_edit_range);
set_custom_hook(app, HookID_BufferRegion, default_buffer_region);
set_custom_hook(app, HookID_ViewChangeBuffer, default_view_change_buffer);
set_custom_hook(app, HookID_Layout, layout_unwrapped);
```

Most of these are self explenetory and just stepping through with a debugger will explain the ones unintelligable on a glance. Some hooks occur on certain stages thus have some implicit behavior they expect. e.g. HookID_RenderCaller expects drawing to occur inside where as HookID_NewFile does not. 

# General Organization

To customize the behavior inside of hooks it is helpful to understand the core ideas you will encounter repeatedly on most customizations. I will now give a general overview of such concepts. 

4coder organizes information through buffers, and views. Buffers are named and hold the textual data (utf8), that can be manipulated in any way you desire. Views on the other hand, are interactive areas on the screen that handle input, and rendering. 

The relationship can be seen as the following: 

Buffer: 
  HI          <-- just text

View:
  +----+      
  | HI |      <--  on the screen and you can manipulate    
  +----+           it so long as you are inside of it's
                   rectangle

Panels are just a way to organize these views on the screen in a visually structured manner, and are the least interesting.

Panel:             
    P0   P1        
  +----+----+  
  | HI | YO |  <-- P0 is the left panel that holds 
  +----+----+      the view with HI as it's text and 
                   P1 is another panel with a different view
                   (actually there exists root panel that is their parent *GASP*)

A buffer holds the content, a view defines how to interact and visualize it, but they exist independently of one another. You can have buffers without views and views without buffers. 

From the docs[^1]:

"The only relationship that ever exists between a view and a buffer is that a view can set a buffer as it's target for writing, reading, and rendering. Multiple views can target a single buffer. Any view can target any buffer at any time. Closing a buffer in a view leaves the view open, closing a view that targets a buffer leaves the buffer open. All of this is to emphasize that these two types of entities exist absolutely independently and have only a very weak relationship."

## Buffers

Buffers hold utf8 textual data with a gap buffer as the underlying datastructure powering them. 

When you use the commands 'interactive_open' or 'interactive_new' or 'interactive_open_or_new' to open a file, you will create a new buffer with a file association like you would in any other text editor you are familiar with. Opening 'program.c' will create a new buffer called 'program.c' with the text from that file (file name conflicts are resolved).

"Once you open a new buffer with a file association it will be the dedicated buffer for that file from then on, opening the file again will always just bring you back to the buffer you already opened, to create a totally different copy you have to manually make a fresh new buffer and then copy the contents over."

But note that this is not the only type of buffer that exists. Some buffers exist to hold text without any backing. The default behavior in 4coder for these buffers is to be named with two '*' characters. A prime example may be the '*scratch*' buffer. You can do all the text editing operations you might expect, but they will not get saved to a file. Another case is how 4coder uses this type of buffer to hold search results from the commands 'list_all_*' in the '*search*' buffer. 

"To see a list of all open buffers and switch a view to targetting any buffer there is the interactive command 'interactive_switch_buffer'."

In the custom layer commands that are buffer related will begin with buffer_*

## Views 

A View is a target for writing input, and reading input; it serves as the primary way of interacting with the editor. All input events, bindings, commands, go into the active view to be handled as it acts like the main controller of input. Even if you hover over an inactive view and execute a command, input still goes through the active view, and there it is handled. 4coder provides a default hook 'default_view_input_handler' where this behavior is detaild and can be changed. It is a small function I recommend reading just to have a basic understanding of a view input handling.

After the active view finished handling all input, it gives up control back to the main application (like a co-routine it yeilds control back), and changes may then be rendered using 'default_render_caller' hook. This hook is where you render the view screen area. Inside a view you can create a painting application, a planning utility or anything else that uses text and rectangles for that matter (not that you should). All this is to say that it is truly seprate from the buffer which usually will accompany a view. 

For most customizations I have found that just these two hooks are the most valuable to understand well (almost anything you may want can be done in them).

## Memory management 

4coder 4.0.29 introduced memory management features to ease lifetime management of data created and used by the custom layer. Now, if you kill a buffer with managed memory attached, the managed memory is automatically cleaned up by 4coder, the same holds to views. The API allows for even more interesting cases. For a more in depth view, click [here](https://4coder.handmade.network/blog/p/3410-new_features_p1__memory_management_overview)

The problem: 

The 4coder has a hard boundry between management of core resources like buffers and views using an API bountry. This means that in the custom layer you do not have access to the buffer or view underlying structures and can not directly attach anything to them. If in the custom layer you would like to create a panel that is unique per buffer, in the old way you will add memory allocation and deallocation on buffer creation and deletion hooks. 

[1]: https://git.mr4th.com/mr4th-public/4coder/src/commit/b5cc96aa2a2d9ff36c147eedc6a9f60345d21bf3/manual/manual.man
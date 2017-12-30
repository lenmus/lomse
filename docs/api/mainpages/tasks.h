/**
@page tasks-page Interaction with your application GUI

@tableofcontents

@section gui-interaction Interactors and Tasks

The Interactor is the key object to interact with the document (here the name '%Interactor') and, currently, a lot of functionality is programmed for doing the most common tasks for renderization, edition and scores playback. But managing user interaction with your application GUI is something totally dependent on GUI design, platform API and framework used for building your application. As Lomse knows nothing about your application and about the operating sytem used, Lomse cannot directly handle mouse and keyboard events and manage user interaction with your application GUI. But can help to reduce the necessary coding in your application. 

There are certain interaction patterns that are often used in applications. For instance: selecting objects by clicking with the mouse and dragging a <i>selection rectangle</i> on the rendered document; or changing the viewed part of the document by clicking with the mouse and dragging the document. And Lomse provides some support methods for helping your application to manage all these typical interaction patterns and reduce the necessary code in your application. 

In Lomse, user interaction patterns are encoded by objects derived from Task base class. Lomse offers some predefined Tasks, such as TaskDragView (for dragging the %View with the mouse) or TaskSelection (for selecting objects with the mouse). By using these predefined Task objects your application can be relieved of coding the supported interaction patterns.


All %Task derivatives are finite state machines. The events that drive the finite state machine are mouse and keyboard events that are delegated by the %Interactor to the %Task. Indeed, the %Task class consists of little more than a set of pure virtual functions dealing with these events.

The %Task responsibility is to decide on the appropriate action for an event or sequence of events. By changing the Task object associated to the %View your application can decide how Lomse will behave when receiving mouse events. A Task is associated to a %View as a result of invoking method Interactor::switch_task(). Your application must invoke this method as a result of some kind of user action (perhaps a menu choice, a keyboard shortcut, or a click in the appropriate button of a palette or toolbar such as choosing a 'selection tool') that implies the user is starting some interaction protocol.

Once associated to the view, the task will continue to collect events and communicate with the %Document class until its lifecycle ends. This may be as a result of completing its job, or because it was somehow cancelled. Then another task will replace it.

So, this is all you have to do:

-# As a result of some kind of user action, decide the interaction pattern to use. For this invoke method Interactor::switch_task() and specify the Task to use.
-# When receiving a mouse event, pass it to Lomse by invoking the appropriate %Interactor method: 
    Interactor::on_mouse_move(), Interactor::on_mouse_button_down(), Interactor::on_mouse_button_up(),
    Interactor::on_mouse_enter_window() or Interactor::on_mouse_leave_window().
-# When the interaction is finished or cancelled go again to step 1.

And, basically, that is all. Lomse will handle the mouse events and will react to them as appropriate, according to the chosen Task rules. Lomse will draw a selection rectangle, change its geometry and select the objects. Or will drag the View, or will drag the selected handler for an object, or will move the object, etc. During this process, your application will receive Lomse events for repainting the window or other, depending on the case.

Example:

@code
void DocumentWindow::on_tool_selected(int toolType)
{
    //switch Lomse Task, according selected tool

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        switch(toolType)
        {
            case k_tool_data_entry:
                spInteractor->switch_task(TaskFactory::k_task_data_entry);
                break;

            case k_tool_selection:
            default:
                spInteractor->switch_task(TaskFactory::k_task_selection);
        }
    }
}

void DocumentWindow::OnMouseEvent(MouseEvent& event)
{
    //pass the event to Lomse for processing it according current selected Task

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        EventType type = event.GetEventType();
        Point pos = event.GetPosition();
        unsigned flags = get_mouse_flags(event);

        if (type == MOUSE_LEFT_DOWN)
            spInteractor->on_mouse_button_down(pos.x, pos.y, flags);

        else if (type == MOUSE_LEFT_UP)
            spInteractor->on_mouse_button_up(pos.x, pos.y, flags);

        else if (type == MOUSE_RIGHT_DOWN)
            spInteractor->on_mouse_button_down(pos.x, pos.y, flags);

        else if (type == MOUSE_RIGHT_UP)
            spInteractor->on_mouse_button_up(pos.x, pos.y, flags);

        else if (type == MOUSE_MOTION)
            spInteractor->on_mouse_move(pos.x, pos.y, flags);
    }
}

unsigned DocumentWindow::get_mouse_flags(MouseEvent& event)
{
    //Helper method for setting event flags depending on event type
    //and on keys pressed while the mouse event took place

    unsigned flags = 0;
    if (event.MouseLeftDown() || event.MouseLeftUp()) 
        flags |= k_mouse_left;
    else if (event.MouseRightDown() || event.MouseRightUp())
        flags |= k_mouse_right;
    if (event.MouseMiddleDown() || event.MouseMiddleUp())
        flags |= k_mouse_middle;

    if (event.ShiftDown())      flags |= k_kbd_shift;
    if (event.AltDown())        flags |= k_kbd_alt;
    if (event.ControlDown())    flags |= k_kbd_ctrl;
    return flags;
}
@endcode


See <a href="examples.html">Examples</a> for a full application sample.



@section tasks-overview Task objects

As said, all %Task derivatives are finite state machines. The events that drive the finite state machine are mouse and keyboard events that are delegated by the %Interactor to the %Task. Indeed, the %Task class consists of little more than a set of pure virtual functions dealing with these events.

To implement the finite state machine representing a task Lomse uses nested switch-case statements. This type of implementation is more compact and it is easy to understand and to follow the finite state machine without relying on state transition diagrams or tables.

The operation of any of the Tasks state machine is quite simple. When the Task is associated to the %View, its method @a ini_task() is invoked and it places the Task in its initial state and kicks everything off. When, finally, it arrives to the 'Done' state, it remains there forever unless the Task is re-started (by invoking @a init_task() again).

Currently there are several predefined Task derived objects that you can use in your application. More Tasks derived clasess can be created for suporting other interaction patterns, but for now these are the existing ones:

- @b TaskDragView class, for dragging the View (moving it by clicking with the mouse and moving the mouse).

- @b TaskOnlyClicks class. It is the default task when edition mode disabled. User can move mouse, and click on objects (links, buttons, etc.). Left click on one object generates an @a on-click event. Right click does nothing. Move mouse generates mouse-in-out events, but only while no button pressed.

- @b TaskSelection class. It is the default task when no specific task assigned. This task is oriented to initiate actions related to selecting objects: drawing a section rectangle, displaying a contextual menu, generating on-click events, and initiating a 'move object' action.
    - Left click down generates on-click event. When procesing this event, your application can:
        - Switch to TaskSelectionRectangle (requires click down)
        - Select object and switch to TaskMoveObject (requires click down)
        - Switch to TaskSelectText (requires click down)
    - Left click up does nothing.
    - Right click down, selects object and shows contextual menu.
    - Right click up does nothing.

- @b TaskSelectionRectangle class. Its purpose is to draw a selection rectangle and select contained objects.

- @b TaskMoveObject class. To drag an image and, finally, move the object to end point.

- @b TaskMoveHandler class. To drag an object handler.

- @b TaskDataEntry class. For inserting new objects by clicking with the mouse. User moves the mouse and clicks at insertion point. Object to insert is determined by the active tool. Left click generates an insert event. Right click, shows contextual menu. As mouse moves without clicking, it can drag an image representing the object to insert. Also, enter/exit box events are generated, as mouse flies over the different areas. Your application can handle these events and react to them as convenient.


In following subsections the transition tables for these Tasks are documented.
In these transition tables:

- The initial state is always the first one in the table.
- Events not appearing in the tables do no change current state and do not trigger any action.


@subsection task-drag-view TaskDragView class


@b Purpose: To move (scroll, drag) the view
    

<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td>WaitingForFirstPoint                <td>left_down   <td>WaitingForSecondPoint   <td>start_scroll
<tr><td rowspan="2">WaitingForSecondPoint   <td>move_mouse  <td>WaitingForSecondPoint   <td>do_scroll
<tr>                                        <td>left_up     <td>WaitingForFirstPoint    <td>end_scroll
</table>


@remarks
    If start point is equal to end point, user just clicked on a point. In this case a "select object" action could be issued, for more flexibility. [Not implemented]





@subsection task-only-clicks TaskOnlyClicks class

@b Purpose: Default task when edition mode disabled. User can move mouse, and click on objects (links, buttons, etc.). Left click on one object generates on-click event. Right click does nothing. Move mouse generates mouse-in-out events, but only while no button pressed.
    

<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td rowspan="2">WaitingForFirstPoint    <td>left_down   <td>WaitingForPoint2        <td>record_first_point
<tr>                                        <td>move_mouse  <td>WaitingForFirstPoint    <td>mouse_in_out
<tr><td>WaitingForPoint2                    <td>left_up     <td>WaitingForFirstPoint    <td>click_at_point
</table>


@remarks
    None.



@subsection task-selection TaskSelection class

@b Purpose: Default task when no specific task assigned. This task is oriented to initiate actions related to selecting objects: drawing a section rectangle, displaying a contextual menu, generating on-click events, and initiating a 'move object' action:

- Left click down generates on-click event. When procesing this event, your application can:
    - Switch to TaskSelectionRectangle (requires click down)
    - Select object and switch to TaskMoveObject (requires click down)
    - Switch to TaskSelectText (requires click down)
- Left click up does nothing.
- Right click down, selects object and shows contextual menu.
- Right click up does nothing.


<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td rowspan="3">WaitingForFirstPoint    <td>left_down   <td>WaitingForPoint2        <td>record_first_point,
                                                                                            decide_on_switching_task
<tr>                                        <td>right_down <td>WaitingForPoint2         <td>record_first_point
<tr>                                        <td>move_mouse <td>WaitingForFirstPoint     <td>mouse_in_out
<tr><td rowspan="3">WaitingForPoint2        <td>move_mouse <td>WaitingForPoint2         <td>none
<tr>                                        <td>left_up    <td>WaitingForFirstPoint     <td>none
<tr>                                        <td>right_up   <td>WaitingForFirstPoint     <td>select_object_at_first_point,
                                                                                            show_contextual_menu
</table>


@remarks
    Action ``decide_on_switching_task()`` is a decision point for continuing in this Task (doing nothing) or switching to an specific task (TaskSelectionRectangle, TaskMoveObject or TaskSelectText).




@subsection task-selection-rectangle TaskSelectionRectangle class

@b Purpose: To draw a selection rectangle and select contained objects.

<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td rowspan="2">WaitingForPoint2        <td>move_mouse <td>WaitingForPoint2         <td>track_sel_rectangle
<tr>                                        <td>left_up    <td>RequestTaskSwitch        <td>select_objects_or_click,
                                                                                            witch_to_default_task
<tr><td>RequestTaskSwitch                   <td>any        <td>RequestTaskSwitch        <td>switch_to_default_task
</table>


@remarks
    This task expects to start with mouse left button clicked down. Therefore, it only expects mouse move and left click up events.



@subsection task-move-object TaskMoveObject class

@b Purpose: To drag an image and, finally, move the object to end point.

<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td rowspan="2">WaitingForPoint2        <td>move_mouse  <td>WaitingForPoint2        <td>move_drag_image
<tr>                                        <td>left_up     <td>RequestTaskSwitch       <td>move_object_or_click,
                                                                                            switch_to_default_task
<tr><td>RequestTaskSwitch                   <td>any         <td>RequestTaskSwitch       <td>switch_to_default_task
</table>


@remarks
    This task expects to start with mouse left button clicked down. Therefore, it only expects mouse move and left click up events.



@subsection task-move-handler TaskMoveHandler class

@b Purpose: To drag a handler.

<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td rowspan="2">WaitingForPoint2        <td>move_mouse  <td>WaitingForPoint2        <td>move_handler
<tr>                                        <td>left_up     <td>RequestTaskSwitch       <td>move_handler_end_point,
                                                                                            switch_to_default_task
<tr><td>RequestTaskSwitch                   <td>any         <td>RequestTaskSwitch       <td>switch_to_default_task
</table>


@remarks
    This task expects to start with mouse left button clicked down. Therefore, it only expects mouse move and left click up events.



@subsection task-data-entry TaskDataEntry class

@b Purpose: To insert new objects by clicking with the mouse. User moves mouse and clicks at insertion point. Object to insert is determined by active tool. Left click generates an insert event. Right click, shows contextual menu. As mouse moves without clicking, it can drag an image representing the object to insert. Also, enter box/exit box events are generated, as mouse flies over the different areas.
    

<b>State Transition Table:</b>

<table>
<tr><th>Current State                       <th>Event       <th>Next State              <th>Action 
<tr><td rowspan="3">WaitingForFirstPoint    <td>left_down   <td>WaitingForPoint2Left    <td>record_first_point
<tr>                                        <td>right_down  <td>WaitingForPoint2Right   <td>record_first_point
<tr>                                        <td>move_mouse  <td>WaitingForFirstPoint    <td>mouse_drag_image
<tr><td>WaitingForPoint2Left                <td>left_up     <td>WaitingForFirstPoint    <td>insert_object
<tr><td>WaitingForPoint2Right               <td>right_up    <td>WaitingForFirstPoint    <td>show_contextual_menu
</table>


@remarks
    Moving the mouse @a after click_down while waiting for click up doesn't change clicked point (down point) and does not generates mouse-in-out events.


*/

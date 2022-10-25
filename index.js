var index =
[
    [ "Introduction", "page-introduction.html", [
      [ "Lomse philosophy", "page-introduction.html#lomse-philosophy", null ],
      [ "Initializing the Lomse library", "page-introduction.html#overview-doorway", null ],
      [ "The Document and related classes", "page-introduction.html#overview-mvc", null ],
      [ "Displaying documents", "page-introduction.html#overview-rendering", null ],
      [ "Printing documents", "page-introduction.html#overview-print", null ],
      [ "Saving and exporting and documents", "page-introduction.html#overview-export", null ],
      [ "Editing documents", "page-introduction.html#overview-edition", null ],
      [ "The sound engine", "page-introduction.html#overview-sound-engine", null ]
    ] ],
    [ "Rendering documents overview", "page-render-overview.html", [
      [ "How to render a document", "page-render-overview.html#mvc-rendering", null ],
      [ "The Lomse Model-View-Controller", "page-render-overview.html#mvc-overview", null ],
      [ "Redering as SVG code", "page-render-overview.html#rendering-svg", null ],
      [ "Rendering as a bitmap", "page-render-overview.html#rendering-bitmap", null ],
      [ "Lomse library initialization", "page-render-overview.html#page-render-overview-init-lomse", null ],
      [ "Displaying the document", "page-render-overview.html#rendering-display", null ],
      [ "View types", "page-render-overview.html#page-render-overview-viewtypes", null ],
      [ "Controlling what is displayed", "page-render-overview.html#page-render-overview-control", null ],
      [ "Tips for several OSs/frameworks", "page-render-overview.html#page-render-overview-tips", [
        [ "Using Lomse in Qt", "page-render-overview.html#page-render-overview-qt", null ],
        [ "Using Lomse in wxWidgets", "page-render-overview.html#page-render-overview-wxwidgets", null ],
        [ "Using Lomse in JUCE", "page-render-overview.html#page-render-overview-juce", null ],
        [ "Using Lomse in X11", "page-render-overview.html#page-render-overview-x11", null ],
        [ "Using Lomse in MS Windows", "page-render-overview.html#page-render-overview-windows", null ]
      ] ]
    ] ],
    [ "Render in SVG format", "page-render-svg.html", [
      [ "How to render SVG", "page-render-svg.html#svg-rendering", null ],
      [ "SVG formatting options", "page-render-svg.html#rendering-svg-formatting", null ],
      [ "SVG 'id' and 'class' attributes", "page-render-svg.html#rendering-svg-attributes", null ],
      [ "Id for notations that generate many images", "page-render-svg.html#rendering-svg-many-shapes", null ]
    ] ],
    [ "Printing documents overview", "page-printing.html", [
      [ "The print API", "page-printing.html#page-printing-overview", null ],
      [ "Determining print buffer size", "page-printing.html#page-printing-buffer-size", null ],
      [ "Save memory by tiling", "page-printing.html#page-printing-tiles", null ],
      [ "Other ways of printing", "page-printing.html#page-printing-other", null ]
    ] ],
    [ "Editing documents overview", "page-edit-overview.html", [
      [ "How to modify a document", "page-edit-overview.html#edit-overview", null ],
      [ "The high-level API: edition commands", "page-edit-overview.html#edit-high-level", [
        [ "Undo/Redo", "page-edit-overview.html#edit-overview-undo", null ],
        [ "Cursor and selections", "page-edit-overview.html#edit-overview-cursor", null ]
      ] ],
      [ "Supported edition modes", "page-edit-overview.html#edit-modes", null ],
      [ "Edition commands", "page-edit-overview.html#edit-commands", null ]
    ] ],
    [ "Saving and exporting documents", "document-export.html", null ],
    [ "Scores playback overview", "page-sound-generation.html", [
      [ "How Lomse playback works", "page-sound-generation.html#page-sound-generation-overview", null ],
      [ "Your application set-up: summary", "page-sound-generation.html#page-sound-generation-summary", null ],
      [ "How to handle sound events", "page-sound-generation.html#page-sound-generation-events", null ],
      [ "How to play an score", "page-sound-generation.html#page-sound-generation-play-score", null ],
      [ "Handling visual tracking events", "page-sound-generation.html#page-sound-generation-tracking", null ],
      [ "The PlayerGui object", "page-sound-generation.html#page-sound-generation-player-gui", null ],
      [ "Using an external player", "page-sound-generation.html#page-sound-generation-external-player", null ]
    ] ],
    [ "Interaction with your application GUI", "page-tasks.html", [
      [ "Interactors and Tasks", "page-tasks.html#gui-interaction", null ],
      [ "Task objects", "page-tasks.html#tasks-overview", [
        [ "TaskDragView class", "page-tasks.html#task-drag-view", null ],
        [ "TaskOnlyClicks class", "page-tasks.html#task-only-clicks", null ],
        [ "TaskSelection class", "page-tasks.html#task-selection", null ],
        [ "TaskSelectionRectangle class", "page-tasks.html#task-selection-rectangle", null ],
        [ "TaskMoveObject class", "page-tasks.html#task-move-object", null ],
        [ "TaskMoveHandler class", "page-tasks.html#task-move-handler", null ],
        [ "TaskDataEntry class", "page-tasks.html#task-data-entry", null ]
      ] ]
    ] ],
    [ "The Document API", "page-api-internal-model.html", [
      [ "Accesing and modifying the document", "page-api-internal-model.html#api-internal-model-intro", null ],
      [ "The ADocument class", "page-api-internal-model.html#api-internal-model-adocument", [
        [ "ADocument content", "page-api-internal-model.html#api-internal-model-adocument-content", null ]
      ] ],
      [ "The structure of a music score", "page-api-internal-model.html#api-internal-model-scores", null ]
    ] ],
    [ "The Graphical Model", "page-graphical-model.html", [
      [ "Structure of the Graphical Model", "page-graphical-model.html#graphical-model-intro", null ],
      [ "Traversing the Graphical Model", "page-graphical-model.html#graphical-model-traversing", null ],
      [ "Drawing bounding boxes", "page-graphical-model.html#graphical-model-boxes", null ],
      [ "Scrolling the View", "page-graphical-model.html#graphical-model-scroll", null ]
    ] ],
    [ "File formats supported by Lomse", "page-file-formats.html", [
      [ "Supported file formats", "page-file-formats.html#page-file-formats-overview", [
        [ "MusicXML format", "page-file-formats.html#mxl-format", null ],
        [ "LDP format", "page-file-formats.html#ldp-format", null ],
        [ "LMD format", "page-file-formats.html#lmd-format", null ]
      ] ]
    ] ],
    [ "Coordinate systems, units, scaling and viewport", "page-coordinates-viewport.html", [
      [ "Device and logical units", "page-coordinates-viewport.html#lomse-units", null ],
      [ "Scaling", "page-coordinates-viewport.html#scale-factor", null ],
      [ "Scrolling: the Viewport", "page-coordinates-viewport.html#viewport-concept", null ]
    ] ],
    [ "How Lomse callbacks work", "page-callbacks.html", null ],
    [ "Events and Requests", "page-events.html", [
      [ "Notifications: events and requests", "page-events.html#notifications", null ],
      [ "Events and how to handle them", "page-events.html#handling-events", null ],
      [ "Events and how are they notified", "page-events.html#events-list", null ],
      [ "How to register an event handler", "page-events.html#event-handlers", null ],
      [ "Handling requests", "page-events.html#handling-requests", null ]
    ] ],
    [ "The logging system", "page-logging.html", [
      [ "Logging disabling and customization", "page-logging.html#logging-overview", null ],
      [ "Logging methods", "page-logging.html#logging-methods", null ],
      [ "Logging areas and messages selection", "page-logging.html#logging-areas", null ]
    ] ]
];
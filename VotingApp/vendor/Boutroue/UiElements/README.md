QtQuick UI Elements
===================

_A consistent set of UI elements with modern look-and-feel, and customizable behavior, to develop UI faster..._


## Controls

These are simple elements that can be used to create forms :

* `TextLabel` : a simple `Text` item, but which follows global `Style` by default (font family, font size, etc...)

* `TextButton` : a simple push button that can have either a label, an icon, or both. Additionally, it can have a custom back color.

* `TextBox` : a single-line editable text-field, which is in fact a wrapper around `TextInput`.

* `MultiLineTextBox` : a multi-line editable text-area, which is in fact a wrapper around `TextEdit`. It has fixed width and, automatic line-wrapping, and vertical auto-size by default.

* `NumberBox` : a composite field that can be used to edit numerical values between min and max bounds. It supports decimals, custom step value, and +/- buttons (that can be triggered with up/down arrow key strokes).

* `CheckableBox` : a simple ON/OFF field.

* `SliderBar` : a bar with movable handle to adjust numerical value in a given min/max range.

* `ProgressJauge` : a simple jauge to show progress of a value in a min/max range.

* `ComboList` : a selector for choosing one key/value pair in a fixed list of proposals. It handles separately the display value and selection key. It uses specific and customizable delegates to be able to use different kinds of models (QML ListModel, JS Array, with custom role names...).

* `Calendar` : a nice and minimalistic date selector using a monthly days grid along with month and year selectors.

* `TimePicker` : a time picker using format `HH:MM:SS`, with separated fields for each sub-part, allowing distinct input validation and up/down buttons (with arrow keys mapping).


## Containers

These elements are used as parents for controls :

* `ScrollContainer` : put a `Flickable` (or derived, e.g. `ListView`) in it to get vertical/horizontal scrollbars displayed around it (according to the flicking direction axis that are set). Those scrollbars can also be turned into simple indicators if needed.

* `Group` : a very basic invisible container for controls, with a title. It does nothing particular by itself, but it is meant to be used as children for `TabBar` and `Accordion` components.

* `TabBar` : a simple container that displays tabs to switch between the `Group` children.

* `Accordion` : another container that wraps/unwraps `Group` children when clicking on headers. Useful for sidebars.

* `ToolBar` : a simple horizontal set of controls that will be positioned inside a bar at top of a page.

* `StatusBar` : a simple horizontal set of controls that will be positioned inside a bar at bottom of a page.

* `PanelContainer` : a container that must be docked to a side of its parent, and that can be detached (in separated sub-window), resized (with a draggable handle), or collasped/expanded. All these features can be separately toggled. It also has support for title and icon.

* `ZoomAndMoveArea` : an easy-to-use view that can be dragged in X/Y axis and zoomed under cursor using mouse-wheel (just like the Google Maps central view). It also strouts nice indicators of positions and zoom level while moving or zooming (they disappear after a while). The wheel sensitivity is adjustable, and zoom min/max boundaries too.


## Layouts

These are invisible items that resize and reposition their children according to predefined behavior :

* `StretchRowContainer` : an horizontal layout, that positions its childrens side-by-side, setting their width in consequence of their implicit width hint, and using remaining space in the layout to distribute it between all items that expose a negative implicit width.

* `StretchColumnContainer` : an vertical layout, that positions its childrens one under the other, setting their height in consequence of their implicit height hint, and their width to its own width, and using remaining space in the layout to distribute it between all items that expose a negative implicit height.

* `GridContainer` : a smart grid that dimensions itself according to the sum/max of its children's implicit size hints, and then distributes regularly the available space between all children, positioned against a column/row model.

* `WrapLeftRightContainer` : a simplified layout for one of the most common positioning scheme in UI : on the same line, put some items to the left, the others to the right. But it has extra intelligency, to wrap itslef it left/right items do not fit in the provided space. It uses a `WrapBreaker` item to separate the left and right items.

* `Stretcher` : a simple item with implicit size hints to `-1` so that `Stretch***Container` auto-size it. It can also contain other items, to stretch easily some items which can't have their implicit size hints overwritten.


## Shapes

These are simple or more complex shapes that can be used in various places :

* `Line` : a thin gray line that can be stretched in vertical or horizontal direction, to be used as a single-side border on a container, or simply as a separator.

* `Polygon` : a simple QtQuick item that takes a list of points and draw them with a provided fill color and stroke width, either as a closed or open shape.

* `RegularPolygon` : a variant of `Polygon` with all sides having same size, and sized using handy diameter instead of side size.

* `RoundedRectangle` : a custom rectangle shape that can have different border-size for each side, and different radius for each corner, like in CSS, because the default `Rectangle` still can't.

* `AbstractSymbol` and `SymbolLoader` : two complementary components, providing resizable and recolorable symbols to use in custom components or as icon for `TextButton`. Some symbols (e.g: cross, plus, minus, arrows...) are available in `Style`, and can be easily used in a `SymbolLoader`.


## Others

These are the helpers and utilities that don't fit in previous categories :

* `Style` : a global class that contains the most widely used variables (small/normal/big font size, font family, fixed font family, small/normal/big spacing, roundness, line size, etc). By changing this single file in an application, the UI elments will adapt automatically, no need to change each one separately.

* `ExtraAnchors` : an attached object which adds a lot of new convenience anchors (these anchors are only oneliners for things that can be acheived with classic anchors in multiple lines). Basically, it allows to anchors left/right, or top/bottom, or top/left/right, or top/left, etc, in a given object (parent or sibling).

* `AutoRepeatableClicker` : a simple wrapper around `MouseArea` which adds the auto-repeat feature.

* `SvgIconHelper` : a class that takes a SVG file as input, plus size/ratio information, and makes a PNG file in persistant cache as output and affect it to its parent item (usually an `Image`). If additional color information other than transparent is provided, the opaque pixels of the output will be colorized with the given tint.

* `SvgIconLoader` : a simple `Image` with a `SvgIconHelper`.

* `FileSelector` : a nice file selector in plain QtQuick, with support for filtering, and icons according to file-type.

* `ThemeIconProvider` : a new image provider, to get icons from the standard FreeDesktop-compliant theme (just use `image://icon-theme/<freedesktop icon name>`).

* `MimeIconsHelper` : a simple non-visual helper that can return the icon to use for a given MIME-type (either using the SVG icons provided with the lib, or the standard global theme).

* `Balloon` : a nice component that can contain a title, a text block and an image, and can be used to provide help or hints in the UI (e.g: in the side bars, or as temporary OSD...).

* `ModalDialog` : a configurable dialog, with title/message, and standard buttons (OK, Yes/No, Cancel, etc...), which can have custom components added to it (like text input, or file selector or anything else), packaged in a nice event-based API that is easy to use. As a bonus, there is a "shake" effect that can be triggered when wanted (for example if the input is not correct).


## Icons

These are minimalistic/modern-looking icons provided with the library, for easy use in your app, divided in several categories (separate folders). They are used with URL `qrc:///QtQmlTricks/icons/<category>/<name>.svg` :

* Category `actions` : primarly used in toolbars, buttons, dialogs, etc. Contains icons `apply`, `arrow-bottom`, `arrow-down`, `arrow-first`, `arrow-last`, `arrow-left`, `arrow-right`, `arrow-top`, `arrow-up`, `attach`, `cancel`, `chevron-down`, `chevron-left`, `chevron-right`, `chevron-up`, `clear`, `config`, `copy`, `cut`, `delete`, `edit`, `export`, `filter`, `find`, `fullscreen`, `help`, `home`, `import`, `info`, `link`, `lock`, `mark`, `minus`, `new-file`, `ok`, `open`, `paste`, `plus`, `redo`, `refresh`, `restore`, `save`, `shutdown`, `stop`, `trash`, `undo`, `view-grid`, `view-list`, `warning`, `zoom-fit`, `zoom-in`, `zoom-original`, `zoom-out`.

* Category `filetypes` : used for files/folders, for example in `FileSelector`. Contains icons `archive`, `binary`, `code`, `database`, `disk-image`, `document`, `drawing`, `executable`, `file`, `folder-closed`, `folder-documents`, `folder-downloads`, `folder-home`, `folder-images`, `folder-music`, `folder-opened`, `folder-videos`, `image`, `pdf`, `script`, `slideshow`, `sound`, `spreadsheet`, `text`, `video`, `webpage`, `xml`.

* Category `devices` : there is in fact a few devices, but there are also some protocol logos. Contains icons `battery`, `bluetooth`, `camera`, `desktop`, `disc`, `ethernet`, `gamepad`, `gps`, `headphones`, `keyboard`, `laptop`, `microphone`, `mouse`, `printer`, `server`, `smartphone`, `speaker`, `tablet`, `usb`, `usb-pendrive`, `webcam`, `wifi`.

* Category `services` : representation of common forms of communication media, along with the logo of the most famous social services. Contains icons `call`, `chat`, `mail`, `rss`.

* Category `others` : every icon that doesn't fit any category. It contains icons `calendar`, `clock`, `cloud`, `moon`, `pin`, `sun`, `target`, `user`.


> NOTE : If you want to donate, use this link : [![Flattr this](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=thebootroo&url=http://gitlab.unique-conception.org/qt-qml-tricks/qt-quick-ui-elements)

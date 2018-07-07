#### xrolodex.res ####   directory of resource names for `xrolodex'

#### Most users will not need, or care about, the contents of
#### this file!
#### Also, editres can generate the same information in a graphical
     layout.

                   Name                           Class
                   ----                           -----
xrolodex
--------
Application
resources:
                   centerDialogs                  CenterDialogs
                   directoryMask                  DirectoryMask
                   entryDelimiter                 EntryDelimiter
                   findSensitive                  FindSensitive
                   findWraparound                 FindWraparound
                   forceSave                      ForceSave
                   indexColumns                   IndexColumns
                   indexRow                       IndexRow
                   indexRows                      IndexRows
                   indexStayUp                    IndexStayUp
                   minimalDialogs                 MinimalDialogs
                   printCommand                   PrintCommand
                   printFormfeed                  PrintFormfeed
                   printStandardOutput            PrintStandardOutput
                   sortRow                        SortRow
                   viewportColumns                ViewportColumns
                   viewportMenuBar                ViewportMenuBar
                   viewportRows                   ViewportRows

User-interface (widget)
resources:
                   mainWindow                     Form
                   databaseLabel                  Label
                   mainMenuBar                    RowColumn
                   menuFileButton                 CascadeButton
                   menuEditButton                 CascadeButton
                   menuFindButton                 CascadeButton
                   menuSortButton                 CascadeButton
                   menuOptionsButton              CascadeButton
                   menuHelpButton                 CascadeButton
                   menuHelpWindowButton           PushButton
                   menuAboutButton                PushButton
                   menuNewButton                  PushButton
                   menuOpenButton                 PushButton
                   menuSaveButton                 PushButton
                   menuSaveAsButton               PushButton
                   menuPrintButton                PushButton
                   menuExitButton                 PushButton
                   menuInsertButton               PushButton
                   menuInsertAfterButton          PushButton
                   menuCopyButton                 PushButton
                   menuDeleteButton               PushButton
                   menuUndeleteButton             PushButton
                   menuFindEntryButton            PushButton
                   menuIndexButton                PushButton
                   menuSetIndexRowButton          PushButton
                   menuResetIndexButton           PushButton
                   menuModifyIndexButton          PushButton
                   menuAscendingButton            PushButton
                   menuDescendingButton           PushButton
                   menuSetSortRowButton           PushButton
                   menuResetSortButton            PushButton
                   menuModifySortButton           PushButton
                   menuDelimiterButton            PushButton
                   menuResetDelimiterButton       PushButton
                   menuModifyDelimiterButton      PushButton
                   menuInfoHelpButton             PushButton

                   fileSelectionDialog            MessageBox

                   findShell                      topLevelShell
                   findPane                       PanedWindow
                   findControl                    Form
                   findLabel                      Label
                   findText                       Text
                   findToggleBox                  RowColumn
                   findSensitiveButton            ToggleButton
                   findWraparoundButton           ToggleButton
                   findAction                     Form
                   findForwardButton              PushButton
                   findReverseButton              PushButton
                   findDismissButton              PushButton

                   printShell                     topLevelShell
                   printPane                      PanedWindow
                   printControl                   Form
                   printLabel                     Label
                   printText                      Text
                   printToggleBox                 RowColumn
                   printStandardOutputButton      ToggleButton
                   printFormfeedButton            ToggleButton
                   printAction                    Form
                   printCurrentButton             PushButton
                   printAllButton                 PushButton
                   printDismissButton             PushButton

                   helpShell                      topLevelShell
                   helpMainWindow                 PanedWindow
                   helpControl                    Form
                   helpPanedWindow                PanedWindow
                   helpListSW                     ScrolledWindow
                   helpList                       List
                   helpTextSW                     ScrolledWindow
                   helpText                       Text
                   helpAction                     Form
                   helpDismissButton              PushButton

editor/viewport
---------------
                   entryEditWindow (Frame)        Editor (Frame)
                   editBox                        Form
depending on
the value of
viewportMenuBar,
either             menuBar                        RowColumn
or                 popupMenu                      RowColumn

                   editWindow                     ScrolledWindow
                   edit                           Text
                   
                   replaceShell                   topLevelShell
                   entryEditWindow (Frame)        Editor (Frame)
                   replacePane                    PanedWindow
                   replaceControl                 Form
                   replaceLabel                   Label
                   replaceFindBox                 Form
                   replaceReplaceBox              Form
                   replaceFindLabel               Label
                   replaceReplaceLabel            Label
                   replaceFindText                Text
                   replaceReplaceText             Text
                   replaceAction                  Form
                   replaceFindButton              PushButton
                   replaceReplaceButton           PushButton
                   replaceDismissButton           PushButton

popup index
-----------
                   listShell                      topLevelShell
                   listShell (Frame)              ListShell (Frame)
                   listPane                       PanedWindow
                   listPort                       ScrolledWindow
                   listList                       List
                   listAction                     Form
                   listDismissButton              PushButton

control panel
-------------
                   entryControlPanel (Frame)      CtrlPanel (Frame)
                   ctrlBox                        RowColumn
                   separator0                     Separator
                   separator1                     Separator
                   ...
                   First                          PushButton
                   Last                           PushButton
                   ...
                   Index                          PushButton

dialog boxes
------------
                   exititUnsavedDialog (Frame)    Dialog (Frame)
                   openUnsavedDialog (Frame)      Dialog (Frame)
                   overwriteDialog (Frame)        Dialog (Frame)
                   delimiterDialog (Frame)        Dialog (Frame)
                   sortDialog (Frame)             Dialog (Frame)
                   indexDialog (Frame)            Dialog (Frame)
                   messageDialog (Frame)          Dialog (Frame)
                   aboutDialog (Frame)            Dialog (Frame)

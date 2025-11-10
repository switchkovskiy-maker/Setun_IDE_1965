object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'Setun IDE - '#1058#1088#1086#1080#1095#1085#1072#1103' '#1083#1086#1075#1080#1082#1072' '#1087#1086' '#1082#1085#1080#1075#1077' 1965 '#1075#1086#1076#1072
  ClientHeight = 699
  ClientWidth = 996
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  Menu = MainMenu
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  OnResize = FormResize
  TextHeight = 13
  object ToolPanel: TPanel
    Left = 0
    Top = 0
    Width = 996
    Height = 30
    Align = alTop
    TabOrder = 0
    object btnConnectionMode: TSpeedButton
      Left = 250
      Top = 0
      Width = 100
      Height = 25
      AllowAllUp = True
      GroupIndex = 1
      Caption = #1057#1086#1077#1076#1080#1085#1077#1085#1080#1103
      OnClick = btnConnectionModeClick
    end
    object btnRunSimulation: TButton
      Left = 0
      Top = 0
      Width = 75
      Height = 25
      Caption = #1057#1080#1084#1091#1083#1103#1094#1080#1103
      TabOrder = 0
      OnClick = btnRunSimulationClick
    end
    object btnResetSimulation: TButton
      Left = 75
      Top = 0
      Width = 75
      Height = 25
      Caption = #1057#1073#1088#1086#1089
      TabOrder = 1
      OnClick = btnResetSimulationClick
    end
    object btnClearWorkspace: TButton
      Left = 150
      Top = 0
      Width = 100
      Height = 25
      Caption = #1054#1095#1080#1089#1090#1080#1090#1100' '#1074#1089#1077
      TabOrder = 2
      OnClick = btnClearWorkspaceClick
    end
  end
  object LibraryPanel: TPanel
    Left = 0
    Top = 30
    Width = 250
    Height = 651
    Align = alLeft
    TabOrder = 1
    ExplicitHeight = 631
    object lblLibrarySelector: TLabel
      Left = 1
      Top = 1
      Width = 74
      Height = 16
      Align = alTop
      Caption = #1041#1080#1073#1083#1080#1086#1090#1077#1082#1072':'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object LibraryLabel: TLabel
      Left = 1
      Top = 38
      Width = 155
      Height = 16
      Align = alTop
      Caption = #1041#1080#1073#1083#1080#1086#1090#1077#1082#1072' '#1101#1083#1077#1084#1077#1085#1090#1086#1074
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object cmbLibrarySelector: TComboBox
      Left = 1
      Top = 17
      Width = 248
      Height = 21
      Align = alTop
      Style = csDropDownList
      TabOrder = 0
      OnChange = cmbLibrarySelectorChange
    end
    object ElementLibrary: TListBox
      Left = 1
      Top = 54
      Width = 248
      Height = 596
      Align = alClient
      ItemHeight = 13
      TabOrder = 1
      OnDblClick = ElementLibraryDblClick
    end
  end
  object WorkspacePanel: TPanel
    Left = 250
    Top = 30
    Width = 746
    Height = 651
    Align = alClient
    TabOrder = 2
    ExplicitHeight = 631
    object SchemePageControl: TPageControl
      Left = 1
      Top = 1
      Width = 748
      Height = 650
      Align = alClient
      OwnerDraw = True
      PopupMenu = TabPopupMenu
      TabOrder = 0
      OnChange = SchemePageControlChange
      OnDrawTab = SchemePageControlDrawTab
      ExplicitWidth = 744
      ExplicitHeight = 629
    end
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 681
    Width = 996
    Height = 18
    Panels = <
      item
        Width = 800
      end>
  end
  object ElementPopupMenu: TPopupMenu
    Left = 400
    Top = 200
    object miProperties: TMenuItem
      Caption = #1057#1074#1086#1081#1089#1090#1074#1072
      OnClick = miPropertiesClick
    end
    object miRotateElement: TMenuItem
      Caption = #1055#1086#1074#1077#1088#1085#1091#1090#1100
      OnClick = miRotateElementClick
    end
    object miViewSubCircuit: TMenuItem
      Caption = #1055#1088#1086#1089#1084#1086#1090#1088
      OnClick = miViewSubCircuitClick
    end
    object miGroupElements: TMenuItem
      Caption = #1043#1088#1091#1087#1087#1080#1088#1086#1074#1072#1090#1100
      OnClick = btnGroupElementsClick
    end
    object miUngroupElements: TMenuItem
      Caption = #1056#1072#1079#1075#1088#1091#1087#1087#1080#1088#1086#1074#1072#1090#1100
      OnClick = btnUngroupElementsClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object miDeleteElement: TMenuItem
      Caption = #1059#1076#1072#1083#1080#1090#1100
      OnClick = miDeleteElementClick
    end
  end
  object SaveDialog: TSaveDialog
    DefaultExt = 'setun'
    Filter = 'Setun Scheme Files (*.setun)|*.setun|All files (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 480
    Top = 200
  end
  object OpenDialog: TOpenDialog
    DefaultExt = 'setun'
    Filter = 'Setun Scheme Files (*.setun)|*.setun|All files (*.*)|*.*'
    Left = 560
    Top = 200
  end
  object MainMenu: TMainMenu
    Left = 640
    Top = 200
    object miFile: TMenuItem
      Caption = #1060#1072#1081#1083
      object miSave: TMenuItem
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100
        ShortCut = 16467
        OnClick = btnSaveSchemeClick
      end
      object miLoad: TMenuItem
        Caption = #1047#1072#1075#1088#1091#1079#1080#1090#1100
        ShortCut = 16460
        OnClick = btnLoadSchemeClick
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object miExit: TMenuItem
        Caption = #1042#1099#1093#1086#1076
        OnClick = miExitClick
      end
    end
    object miEdit: TMenuItem
      Caption = #1055#1088#1072#1074#1082#1072
      object miGroup: TMenuItem
        Caption = #1043#1088#1091#1087#1087#1080#1088#1086#1074#1072#1090#1100
        ShortCut = 16455
        OnClick = btnGroupElementsClick
      end
      object miUngroup: TMenuItem
        Caption = #1056#1072#1079#1075#1088#1091#1087#1087#1080#1088#1086#1074#1072#1090#1100
        ShortCut = 16469
        OnClick = btnUngroupElementsClick
      end
    end
    object miView: TMenuItem
      Caption = #1042#1080#1076
      object miZoomIn: TMenuItem
        Caption = #1059#1074#1077#1083#1080#1095#1080#1090#1100
        ShortCut = 16424
        OnClick = btnZoomInClick
      end
      object miZoomOut: TMenuItem
        Caption = #1059#1084#1077#1085#1100#1096#1080#1090#1100
        ShortCut = 16422
        OnClick = btnZoomOutClick
      end
      object miZoomFit: TMenuItem
        Caption = #1055#1086#1076#1086#1075#1085#1072#1090#1100
        ShortCut = 16454
        OnClick = btnZoomFitClick
      end
    end
    object miOptions: TMenuItem
      Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1080
      object miRectangularConnections: TMenuItem
        Caption = #1055#1088#1103#1084#1086#1091#1075#1086#1083#1100#1085#1099#1077' '#1089#1086#1077#1076#1080#1085#1077#1085#1080#1103
        OnClick = miRectangularConnectionsClick
      end
      object miSnapToGrid: TMenuItem
        Caption = #1055#1088#1080#1074#1103#1079#1082#1072' '#1082' '#1089#1077#1090#1082#1077
        Checked = True
        OnClick = miSnapToGridClick
      end
    end
    object miSimulation: TMenuItem
      Caption = #1057#1080#1084#1091#1083#1103#1094#1080#1103
      object miRun: TMenuItem
        Caption = #1047#1072#1087#1091#1089#1090#1080#1090#1100
        ShortCut = 120
        OnClick = btnRunSimulationClick
      end
      object miReset: TMenuItem
        Caption = #1057#1073#1088#1086#1089
        ShortCut = 116
        OnClick = btnResetSimulationClick
      end
    end
    object miHelp: TMenuItem
      Caption = #1057#1087#1088#1072#1074#1082#1072
      object miAbout: TMenuItem
        Caption = #1054' '#1087#1088#1086#1075#1088#1072#1084#1084#1077
        OnClick = miAboutClick
      end
    end
  end
  object TabPopupMenu: TPopupMenu
    Left = 720
    Top = 200
    object miCloseTab: TMenuItem
      Caption = #1047#1072#1082#1088#1099#1090#1100' '#1074#1082#1083#1072#1076#1082#1091
      OnClick = miCloseTabClick
    end
  end
end

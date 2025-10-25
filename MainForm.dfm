object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'Setun IDE - '#1058#1088#1086#1080#1095#1085#1072#1103' '#1083#1086#1075#1080#1082#1072' '#1087#1086' '#1082#1085#1080#1075#1077' 1965 '#1075#1086#1076#1072
  ClientHeight = 700
  ClientWidth = 1000
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  TextHeight = 13
  object ToolPanel: TPanel
    Left = 0
    Top = 0
    Width = 1000
    Height = 50
    Align = alTop
    TabOrder = 0
    ExplicitWidth = 996
    object MainToolBar: TToolBar
      Left = 1
      Top = 1
      Width = 998
      Height = 48
      ButtonHeight = 44
      ButtonWidth = 45
      Caption = 'MainToolBar'
      ShowCaptions = True
      TabOrder = 0
      ExplicitWidth = 994
      object btnRunSimulation: TButton
        Left = 0
        Top = 0
        Width = 75
        Height = 44
        Caption = #1057#1080#1084#1091#1083#1103#1094#1080#1103
        TabOrder = 0
        OnClick = btnRunSimulationClick
      end
      object btnResetSimulation: TButton
        Left = 75
        Top = 0
        Width = 75
        Height = 44
        Caption = #1057#1073#1088#1086#1089
        TabOrder = 1
        OnClick = btnResetSimulationClick
      end
      object btnClearWorkspace: TButton
        Left = 150
        Top = 0
        Width = 100
        Height = 44
        Caption = #1054#1095#1080#1089#1090#1080#1090#1100' '#1074#1089#1077
        TabOrder = 2
        OnClick = btnClearWorkspaceClick
      end
      object btnConnectionMode: TSpeedButton
        Left = 250
        Top = 0
        Width = 100
        Height = 44
        AllowAllUp = True
        GroupIndex = 1
        Caption = #1057#1086#1077#1076#1080#1085#1077#1085#1080#1103
        OnClick = btnConnectionModeClick
      end
      object btnZoomIn: TButton
        Left = 350
        Top = 0
        Width = 75
        Height = 44
        Caption = #1059#1074#1077#1083#1080#1095#1080#1090#1100
        TabOrder = 3
        OnClick = btnZoomInClick
      end
      object btnZoomOut: TButton
        Left = 425
        Top = 0
        Width = 75
        Height = 44
        Caption = #1059#1084#1077#1085#1100#1096#1080#1090#1100
        TabOrder = 4
        OnClick = btnZoomOutClick
      end
      object btnZoomFit: TButton
        Left = 500
        Top = 0
        Width = 75
        Height = 44
        Caption = #1055#1086#1076#1086#1075#1085#1072#1090#1100
        TabOrder = 5
        OnClick = btnZoomFitClick
      end
    end
  end
  object LibraryPanel: TPanel
    Left = 0
    Top = 50
    Width = 250
    Height = 632
    Align = alLeft
    TabOrder = 1
    ExplicitHeight = 631
    object LibraryLabel: TLabel
      Left = 1
      Top = 1
      Width = 257
      Height = 16
      Align = alTop
      Caption = #1041#1080#1073#1083#1080#1086#1090#1077#1082#1072' '#1101#1083#1077#1084#1077#1085#1090#1086#1074' ('#1057#1077#1090#1091#1085#1100', 1965)'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object ElementLibrary: TListBox
      Left = 1
      Top = 17
      Width = 248
      Height = 614
      Align = alClient
      ItemHeight = 13
      TabOrder = 0
      OnDblClick = ElementLibraryDblClick
    end
  end
  object WorkspacePanel: TPanel
    Left = 250
    Top = 50
    Width = 750
    Height = 632
    Align = alClient
    TabOrder = 2
    ExplicitWidth = 746
    ExplicitHeight = 631
    object Workspace: TScrollBox
      Left = 1
      Top = 1
      Width = 748
      Height = 630
      Align = alClient
      TabOrder = 0
      OnResize = WorkspaceResize
      ExplicitWidth = 744
      ExplicitHeight = 629
      object CircuitImage: TPaintBox
        Left = 0
        Top = 0
        Width = 744
        Height = 626
        OnMouseDown = CircuitImageMouseDown
        OnMouseMove = CircuitImageMouseMove
        OnMouseUp = CircuitImageMouseUp
        OnPaint = CircuitImagePaint
      end
    end
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 682
    Width = 1000
    Height = 18
    Panels = <
      item
        Width = 800
      end>
    ExplicitTop = 681
    ExplicitWidth = 996
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
    object N1: TMenuItem
      Caption = '-'
    end
    object miDeleteElement: TMenuItem
      Caption = #1059#1076#1072#1083#1080#1090#1100
      OnClick = miDeleteElementClick
    end
  end
end

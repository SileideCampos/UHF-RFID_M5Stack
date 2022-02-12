unit UPrincipal;

interface

uses
  System.SysUtils, System.Types, System.UITypes, System.Classes, System.Variants,
  FMX.Types, FMX.Controls, FMX.Forms, FMX.Graphics, FMX.Dialogs, FMX.StdCtrls,
  FMX.Controls.Presentation, FMX.Edit, IdBaseComponent, IdComponent, IdUDPBase,
  IdUDPClient, FMX.Layouts, FMX.ListBox;

type
  TFRFID = class(TForm)
    udp: TIdUDPClient;
    edtLeitura: TEdit;
    ListBox1: TListBox;
    ListBox2: TListBox;
    Layout1: TLayout;
    btnConexao: TSwitch;
    edtIP: TEdit;
    Layout2: TLayout;
    Layout3: TLayout;
    Label1: TLabel;
    btnLimpar: TButton;
    procedure conexao;
    procedure btnConexaoSwitch(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure ListBox1DblClick(Sender: TObject);
    procedure btnLimparClick(Sender: TObject);
  private
    FThread: TThread;
    valorTemp: String;
    procedure finalizarThread;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FRFID: TFRFID;

implementation

{$R *.fmx}

procedure TFRFID.btnLimparClick(Sender: TObject);
begin
 ListBox1.Clear;
 ListBox2.Clear;
end;

procedure TFRFID.conexao;
begin
  udp.Host := edtIP.Text;
  udp.Port := 3333;
  try
    udp.Connect;
  except
    ShowMessage('Conexão recusada. Verifique e tente novamente!');
    Exit;
  end;
end;

procedure TFRFID.btnConexaoSwitch(Sender: TObject);
begin
  if btnConexao.IsChecked then
  begin
    conexao;
    FThread := TThread.CreateAnonymousThread(procedure
      var
        lLeitura: string;
        lExiste: Boolean;
      begin
        while not TThread.CheckTerminated do
        begin
          if udp.Connected then
          begin
            udp.Send('1');
            lLeitura := udp.ReceiveString(700);
            sleep(100);
            TThread.Synchronize(nil,
              procedure
              begin
                if lLeitura <> '' then
                begin
                  edtLeitura.Text := lLeitura;
                  lExiste := False;
                  for var li in ListBox1.Items do
                  begin
                    if li = lLeitura then
                      lExiste := True;
                  end;
                  if not lExiste then
                    ListBox1.Items.Add(lLeitura);
                  ListBox2.Items.Add(lLeitura);
                end;
              end
            );
          end
        end;
      end);
    FThread.FreeOnTerminate := false;
    FThread.Start;
  end;
end;

procedure TFRFID.finalizarThread;
begin
  if FThread <> nil then
  begin
    FThread.Terminate;
    FThread.WaitFor;
    FreeAndNil(FThread);
  end;
end;

procedure TFRFID.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  finalizarThread;
end;

procedure TFRFID.ListBox1DblClick(Sender: TObject);
begin
  ListBox1.Clear;
end;

end.

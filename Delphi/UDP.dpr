program UDP;

uses
  System.StartUpCopy,
  FMX.Forms,
  UPrincipal in 'UPrincipal.pas' {FRFID};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TFRFID, FRFID);
  Application.Run;
end.

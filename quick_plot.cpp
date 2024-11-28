#include "TF1.h"
#include "TCanvas.h"

int main(){

  TF1 *f1 = new TF1(
		    "sin(x)",
		    "sin(x)",
		    -5,5);

  TCanvas * c1 = new TCanvas();
  f1->Draw();
  c1->Print("sin.png");

  return 0;
}

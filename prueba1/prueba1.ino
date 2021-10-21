void serial(void);
unsigned int S=0;
unsigned int SS=0;
unsigned int C=54;
unsigned int CC=49;
unsigned int x=0;
void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  pinMode(PA_2, INPUT_PULLUP);
  pinMode(PA_3, INPUT_PULLUP);
  pinMode(PA_4, INPUT_PULLUP);
  pinMode(PA_5, INPUT_PULLUP);
  pinMode(PD_2, INPUT_PULLUP);
  pinMode(PD_3, INPUT_PULLUP);
  pinMode(PD_1, INPUT_PULLUP);
  pinMode(PD_0, INPUT_PULLUP);
}

void loop() {
  if (Serial3.available()){
    x=Serial3.read();
    if (x==1){
      Serial3.write(C);
      Serial3.write(CC);
      Serial3.write(10);
    }
  }
  serial();
}


void serial(void){
  if ((digitalRead(PA_5)==HIGH && digitalRead(PA_3)==HIGH && digitalRead(PA_2)==HIGH && digitalRead(PA_4)==LOW&&(S!=2))){
    S=2;
    C=54;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  else if (digitalRead(PA_5)==LOW && digitalRead(PA_4)==HIGH && digitalRead(PA_3)==HIGH && digitalRead(PA_2)==HIGH && (S!=1)){
    S=1;
    C=53;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  else if (digitalRead(PA_3)==LOW && digitalRead(PA_2)==HIGH && digitalRead(PA_4)==HIGH && digitalRead(PA_5)==HIGH && (S!=3)){
    S=3;
    C=55;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  else if (digitalRead(PA_3)==HIGH && digitalRead(PA_4)==HIGH && digitalRead(PA_5)==HIGH && digitalRead(PA_2)==LOW && (S!=4)){
    S=4;
    C=56;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  if (digitalRead(PD_2)==HIGH && digitalRead(PD_3)==HIGH && digitalRead(PD_1)==HIGH && digitalRead(PD_0)==LOW&&(SS!=1)){
    SS=1;
    CC=49;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  else if (digitalRead(PD_2)==HIGH && digitalRead(PD_3)==HIGH && digitalRead(PD_0)==HIGH && digitalRead(PD_1)==LOW && (SS!=2)){
    SS=2;
    CC=50;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  else if (digitalRead(PD_0)==HIGH && digitalRead(PD_3)==HIGH && digitalRead(PD_1)==HIGH && digitalRead(PD_2)==LOW && (SS!=3)){
    SS=3;
    CC=51;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
  else if (digitalRead(PD_2)==HIGH && digitalRead(PD_0)==HIGH && digitalRead(PD_1)==HIGH && digitalRead(PD_3)==LOW && (SS!=4)){
    SS=4;
    CC=52;
    Serial3.write(C);
    Serial3.write(CC);
    Serial3.write(10);
  }
}

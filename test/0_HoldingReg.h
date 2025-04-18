uint16_t lastSVal;
uint16_t HVal1;
uint16_t HoldregSet(TRegister* reg, uint16_t val) {
  if (lastSVal != val) {
    lastSVal = val;
    Serial.println(String("HregSet val:") + String(val));
  }

  //HVal1 = val;
  //mb.Hreg(D1, val);  // เขียนข้อมูลไป Address 1 ของ  Holding register
  return val;
}


//Holding register 0 อ่านฟังก์ชันเรียกกลับเหตุการณ์ข้อมูล Address 0
uint16_t D0HoldregGet (TRegister* reg, uint16_t val0) {
  D_0 = val0;
  Serial.print("D0 = "); Serial.println(D_0);
  return val0;
}

//Holding register อ่านฟังก์ชันเรียกกลับเหตุการณ์ข้อมูล Address 1
uint16_t D1HoldregGet (TRegister* reg, uint16_t val1) {
  D_1 = val1;
  Serial.print("D1 = "); Serial.println(D_1);
  return val1;
}




//----------------------------------------------------------------------------------------------------------

//coil  Address 0
uint16_t cbCoilSet(TRegister* reg, uint16_t val) {

  //Serial.println(String("CoilSet1 val: ") + String(val));

  return val;
}

//coil Address 0
uint16_t M0CoilGet(TRegister* reg, uint16_t val0) {
  M_0 = val0;
  Serial.print("M0 = "); Serial.println(M_0);
  return val0;
}
//coil Address 1
uint16_t M1CoilGet(TRegister* reg, uint16_t val1) {
  M_1 = val1;
  Serial.print("M1 = "); Serial.println(M_1);
  return val1;
}

#include "RemoteLight.h"

std::shared_ptr<remoteLight::RemoteLight> rml =std::make_shared<remoteLight::RemoteLight>();

void setup() {
  rml->init();
}

void loop() {
  rml->run();
  delay(10);
}
/*Este programa emplea un Funduino UNO, un RTC "Tiny RTC", la microsd shield de sparkfun y el sensor de temperatura 
del grove kit de seeed studio.

Funcionamiento:
Este programa permite que el sensor mida la temperatura ambiente y lea la fecha y hora del RTC, cada un intervalo de
tiempo en minutos que debemos definir en el propio programa antes de cargarlo al arduino, una vez medida la temperatura
y la fecha y hora nos los guarda en la microsd y nos lo pinta por serial, si no tenemos una microsd no haría nada, por 
serial avisaría de que hay un error con la tarjeta

IMPORTANTE: sin tarjeta aunque queranos mostrar por serial el tiempo y temperatura
el tiempo no se muestra correctamente*/

#include <Wire.h>
#include <math.h>
#include <SD.h>

#define DS1307_I2C_ADDRESS 0x68
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
float temperatura, resistencia, interval=0.06, prev_mill=0, time=0, sensor;
int a, B=3975, vector[8],i, index=0;
const int chipSelect = 8;

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// Lee la fecha y hora del reloj
void getDateDs1307(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month,byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
 
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
 
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f); //formato 24h
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}
 


//inicialización
void setup()
{
  Serial.begin(9600);
  Serial.flush();
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(chipSelect, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.\n\n");
  Wire.begin();
  pinMode(10, OUTPUT); //Pin 10 must be set as an output for the SD communication to work.
}


//función principal
void loop()
{
  time=millis();
  delay(10);
 
  //como millis se resetea al llegar a 50 días tenemos en cuenta eso y si esto pasase reiniciamos tambien 
  //prev_mill, cuando sabemos que se reseteó? Pues cuando millis (tiempo real) no sea mayor al instante donde
  //hemos hecho la última medida de temperatura, en ese caso resetea también prev_mill
  if(millis()>time)
  {
    //pasamos el resultado de time . prev:mill de milisegundos a minutos
    if (((time-prev_mill)/60000)>interval)
    {
      //Cálculo de la temperatura
      a=0;
      for(i=0;i<8;i++)
      {
        vector[i]=analogRead(0);
        a=a+vector[i];
      }
      a=a/8;
      resistencia=(float)(1023-a)*10000/a; 
      temperatura=1/(log(resistencia/10000)/B+1/298.15)-273.15;
      
      //Cálculo de la hora
      getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
      File dataFile = SD.open("datalog.txt", FILE_WRITE);

      // COmprueba que se haya abierto el archivo y si lo está escribe fecha, hora y temperatura
      if (dataFile) 
      {  
        
        dataFile.print( "Fecha: ");
        dataFile.print(dayOfMonth);
        dataFile.print("/");
        dataFile.print(month);
        dataFile.print("/20");
        dataFile.print(year);
        dataFile.print(" Hora ");
        dataFile.print(hour);
        dataFile.print( ":");
        if (minute > 10)
          {
             dataFile.print(minute);
          }
          else
          {
             dataFile.print( "0");
             dataFile.print(minute);
          }
        dataFile.print(" ----------->");
        dataFile.print( " temperatura = ");
        dataFile.print(temperatura,1);
        dataFile.println(" grados C");
        dataFile.close();
        
              //pintamos por serial lo mismo
        Serial.print( "Fecha: ");
        Serial.print(dayOfMonth);
        Serial.print("/");
        Serial.print(month);
        Serial.print("/20");
        Serial.print(year);
        Serial.print(" Hora ");
        Serial.print(hour);
        Serial.print( ":");
        if (minute > 10)
          {
             Serial.print(minute);
          }
          else
          {
             Serial.print( "0");
             Serial.print(minute);
          }
        Serial.print(" ----------->");
        Serial.print( " temperatura = ");
        Serial.print(temperatura,1);
        Serial.println(" grados C");
      }  
      // if the file isn't open, pop up an error:
      else
      {
        Serial.println("error opening datalog.txt");
      } 
        //Guardamos el instante de tiempo en que tomamos la medida para luego obtener cuanto tiempo ha pasado desde el
        //momento actual y el instante donde hicimos la última medida
        prev_mill=time;  
  }
      /*esto solo funciona para hacer la primera medida de temperatura, así independientemente del valor de interval que
      tomemos, al iniciar el arduino tomaremos nuestra primera medida*/
  else
  {
    if (prev_mill==0)
    {
      
      //Cálculo de la temperatura
      a=0;
      for(i=0;i<8;i++)
      {
        vector[i]=analogRead(0);
        a=a+vector[i];
      }
      a=a/8;
      resistencia=(float)(1023-a)*10000/a; 
      temperatura=1/(log(resistencia/10000)/B+1/298.15)-273.15;
      
        //Cálculo de la hora
        getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
        //Cálculo de la temperatura
        a=0;
        for(i=0;i<8;i++)
        {
          vector[i]=analogRead(0);
          a=a+vector[i];
        }
        a=a/8;
        resistencia=(float)(1023-a)*10000/a; 
        temperatura=1/(log(resistencia/10000)/B+1/298.15)-273.15;
       
        File dataFile = SD.open("datalog.txt", FILE_WRITE);
        // if the file is available, write to it:
        if (dataFile) 
        {  
          dataFile.println("\n\n-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-\n");
          dataFile.print( "Fecha: ");
          dataFile.print(dayOfMonth);
          dataFile.print("/");
          dataFile.print(month);
          dataFile.print("/20");
          dataFile.print(year);
          dataFile.print(" Hora ");
          dataFile.print(hour);
          dataFile.print( ":");
          if (minute > 10)
          {
             dataFile.print(minute);
          }
          else
          {
             dataFile.print( "0");
             dataFile.print(minute);
          }
          dataFile.print(" ----------->");
          dataFile.print( " temperatura = ");
          dataFile.print(temperatura,1);
          dataFile.println(" grados C");
          dataFile.close();
          
          Serial.println("\n\n-x-x-x-x-x-x-x-x-x-x-x-x-x-x-\n");
          Serial.print( "Fecha: ");
          Serial.print(dayOfMonth);
          Serial.print("/");
          Serial.print(month);
          Serial.print("/20");
          Serial.print(year);
          Serial.print(" Hora ");
          Serial.print(hour);
          Serial.print( ":");
          if (minute > 10)
          {
             Serial.print(minute);
          }
          else
          {
             Serial.print( "0");
             Serial.print(minute);
          }
          Serial.print(" ----------->");
          Serial.print( " temperatura = ");
          Serial.print(temperatura,1);
          Serial.println(" grados C");
         }
           // if the file isn't open, pop up an error:
        else
        {
          Serial.println("error opening datalog.txt");
        } 
          
      //Le damos un valor ~=0 a prev_mill para que salga del bucle, pues es solo para 
      //que al haga la primera medida al iniciar el arduino
      prev_mill=millis();
    }
  }
 }
 
 else
  {
    time=millis();
    prev_mill=time;
  }
}

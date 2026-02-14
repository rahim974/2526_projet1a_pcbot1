# Diagramme + Bus I2C et Bus SPI

```mermaid
graph LR
    MCU[STM32G431]
    
    subgraph BUS_I2C [Labels: SDA / SCL]
        TOF[Capteur TOF]
        IMU[Accéléromètre LSM6DSOX]
        BQ[Chargeur BQ25896]
    end

    
    subgraph BUS_SPI [Labels: SCK / MISO / MOSI]
        NRF[nRF24L01]
    end

    subgraph POWER [Labels: VCC / GND]
        BAT[Batterie 18650]
        USB[Prise Secteur/USB]
    end


    subgraph MOTORS [Labels: PWM]
        DRV[Driver DRV8411A]
        M1[Moteurs CC]
    end

    MCU -- "I2C" --- BUS_I2C
    MCU -- "SPI" --- BUS_SPI
    MCU -- "PWM (Vitesse)" --- DRV
    DRV --- M1
    
    USB -- "Charge" --- BQ
    BAT -- "Energie" --- BQ
    BQ -- "Alimentation" --- MCU
    

    NRF -. "Radio" .-> Station[Station de Base]
    
```



#### BUS I2C :

https://www.yoctopuce.com/FR/article/petite-introduction-au-bus-i2c

https://youtu.be/15XY4LoQyjc?si=3IJ6fjEveq3ZDlDo

#### BUS SPI :

https://lycrouv-sn.fr/rsc/TdTp/Td/ciel2/Datalink/res/bus_spi.pdf

https://youtu.be/2hnIKzI6yYE?si=DDTpKShhfTgUhP8


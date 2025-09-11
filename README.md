# RI-STM32WLE5-Bootloader_OTA_UART

**Autor:** ManuProjects000  
**Contacto:** martinezleanesmanuel@gmail.com 

Proyecto de **bootloader OTA vía UART** para el microcontrolador **STM32WLE5**.  
Este repositorio contiene el bootloader y archivos personalizados necesarios para que la aplicación principal pueda integrarse correctamente y realizar actualizaciones seguras mediante comunicación UART.

---

## Tabla de contenidos

- [Descripción](#descripcion)  
- [Características](#caracteristicas)
- [Integración con la App](#integracion-con-la-app)  
- [Cómo usarlo](#como-usarlo)   

---

## Descripción

El objetivo de este repositorio es proveer un **bootloader para actualizaciones de firmware vía UART** en dispositivos basados en el **STM32WLE5**.  
La lógica permite recibir un nuevo binario desde un host (ej: PC, gateway o script Python) y escribirlo en la Flash del microcontrolador, asegurando el reemplazo correcto de la aplicación previa.

---

## Características

- Compatible con microcontroladores **STM32WLE5**  
- Comunicación **UART** para la transferencia de firmware  
- Verificación de integridad de datos  
- Estructura modular que facilita su integración en aplicaciones externas  
- Soporte para FreeRTOS (la App debe declarar tareas específicas para el proceso de update)  

---

## Integración con la App

Para que la aplicación principal pueda interactuar con este bootloader es **necesario incluir la carpeta `custom_files/`** en el proyecto de la App.  

Además, dentro de FreeRTOS, la aplicación debe:  

1. **Declarar un `threadHandler`** asociado a la tarea que procesará la actualización.  
2. Asignar el flag `FLAG_RECEIVE_UPDATE` a la tarea `r_receive_update`.  

## Cómo usarlo

1. **Compilar y flashear el bootloader** en la dirección base (`0x08000000`).

2. **Incluir la carpeta `CustomFiles/`** en el proyecto de la App. 

3. **UART:** Modificar puntero declarado en main para asignar la UART por la que se recibe el update en el Bootloader
    - **&huart1**: Comunicacion con el ESP32
    - **&huart2**: Comunicacion via rs232 hacia la PC
      
    ```c
    UART_HandleTypeDef *p_uart = &huart2;
    ```
  
    - Se debe agregar este mismo puntero en el `main.c` de la App principal e inicializarlo en la funcion `main()`:
    
    ```c
    HAL_UART_Receive_IT(p_uart, (uint8_t *)&s_rx_byte, 1);
    ```
    - Se debe declarar la variable s_rx_byte en el `main.c` de la App principal para que el callback funcione:
    
    ```c
    extern volatile uint8_t s_rx_byte;
    ```
 
4. **FreeRTOS:** declarar un `threadHandler` asociado a la tarea que procesará la actualización y asignar el flag `FLAG_RECEIVE_UPDATE` a la tarea `r_receive_update` dentro de `MX_FreeRTOS_Init()`:
   
   ```c
   h_receiveUpdateHandle = osThreadNew(r_receive_update, NULL, &receiveUpdate_attributes);
   ```
   
5. **Ejecutar un script en PC o utlizar el ESP32** para enviar el binario vía UART.  
6. El **bootloader recibirá los datos**, validará la transferencia y programará la nueva versión de la App.

---

### Cambiar ubicaciones en la Flash

En caso de querer mover las aplicaciones a otras direcciones de la flash:

#### Linker
→ Ir a: **Options for Target → Linker**  
- Verificar que la opción **Use Memory Layout from Target Dialog** esté habilitada.  
- En: **Target → Read/Only Memory Areas → IROM1**  
  - Configurar **Start** en la dirección deseada.  
  - Configurar **Size** según el tamaño requerido.  

#### Debug
→ Ir a: **Options for Target → Debug → ST-Link Debugger Settings → Flash Download**  
- Ajustar:  
  - **Address Range**: establecer el límite inferior de la dirección.  
  - **Size**: modificar en caso de que la aplicación requiera un tamaño diferente.  

#### Archivo `stm32wle5xx.h`
→ Abrir `stm32wle5xx.h`  
- Cambiar la definición de la dirección base de la Flash:
  
  ```c
  #define FLASH_BASE   0x080XXXXX   // Dirección de inicio de la aplicación
  ```
  
#### Archivo `system_stm32wlxx.c`
- Comentar esta parte de codigo:
  
  ```c
  #define USER_VECT_TAB_ADDRESS
  ```
  

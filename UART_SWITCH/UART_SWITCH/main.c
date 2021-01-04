#include <atmel_start.h>
#include <string.h>

// Serial Receiving & Complete Flags
volatile uint8_t serial_receiving = 0;
volatile uint8_t serial_complete = 0;

// Bytes Received Counters
volatile uint8_t serial_received_bytes_counter = 0;
volatile uint8_t total_bytes = 0;

// Size of Receive Buffer
#define SERIAL_BUFFER_SIZE	200

// Receive and Transmit Buffers
 uint8_t rx_buffer[SERIAL_BUFFER_SIZE] = { 0x00 };
 uint8_t tx_buffer[SERIAL_BUFFER_SIZE + 14] = "Your message: ";

/**
 * Virtual COM Port Receive Callback Function
 *
 */
static void serial_rx_cb(const struct usart_async_descriptor *const io_descr)
{
	// Counters
	uint8_t ch, count;
	
	// Read a Character
	count = io_read(&USART_0.io, &ch, 1);
	
	// Check if we're receiving
	if (serial_receiving == 0)
	{
		// Check for New Line or Carriage Return
		if (ch != '\r' && ch != '\n')
		{
			// Set Receiving Flag
			serial_receiving = 1;
			
			// Reset Byte Counter
			serial_received_bytes_counter = 0;
			
			// Start Filling the Buffer
			rx_buffer[serial_received_bytes_counter] = ch;
			
			// Increment the Byte Counter
			serial_received_bytes_counter += count;
		}
	}
	else
	{
		// Continue Filling Buffer
		rx_buffer[serial_received_bytes_counter] = ch;
		
		// Increment the Byte Counter
		serial_received_bytes_counter += count;
		
		// Check for New Line or Carriage Return
		if (ch == '\r' || ch == '\n')
		{
			// Set the Completion Flag
			serial_complete = 1;
			
			// Total Bytes
			total_bytes = serial_received_bytes_counter - 2;
		}
		
		// Check for Buffer Overflow
		if (serial_received_bytes_counter >= SERIAL_BUFFER_SIZE)
		{
			// Reset Buffer Counter
			serial_received_bytes_counter = 0;
		}
	}
}

/**
 * Virtual COM Port Transmit Callback Function.
 *
 */
static void serial_tx_cb(const struct usart_async_descriptor *const io_descr)
{
	/* Do Nothing */
}

int main(void)
{
	uint8_t temp;
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	// Initialise ASYNC Driver
	usart_async_register_callback(&USART_0, USART_ASYNC_TXC_CB, serial_tx_cb);
	usart_async_register_callback(&USART_0, USART_ASYNC_RXC_CB, serial_rx_cb);
	usart_async_enable(&USART_0);

	/* Replace with your application code */
	while (1)
	{
		// Check if we're receiving
		if (serial_receiving == 1)
		{
			// Check if we're complete
			if (serial_complete == 1)
			{
				// Reset Flags
				serial_receiving = 0;
				serial_complete = 0;
				
				// Copy Message to TX Buffer
				memcpy(&tx_buffer[14], &rx_buffer[0], SERIAL_BUFFER_SIZE);
				for(int i=0;i<=total_bytes;i++)
				{
					
					temp=tx_buffer[14+i]/100+48;
					io_write(&USART_0.io,&temp,1);
					temp=(tx_buffer[14+i]/10%10)+48;
					io_write(&USART_0.io,&temp,1);
					temp=tx_buffer[14+i]%10+48;
					io_write(&USART_0.io,&temp,1);
				}
				
				// Print a Message
				//io_write(&USART_0.io, tx_buffer, total_bytes + 16);
				gpio_toggle_pin_level(LED);
				
				// Clear Memory
				memset(&rx_buffer, 0x00, SERIAL_BUFFER_SIZE);
			}
		}
	}
}
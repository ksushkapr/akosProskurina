#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/fs.h>
#include <asm/uaccess.h>

// информация о модуле, которую можно будет увидеть с помощью Modinfo
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "KProskurina" );
MODULE_DESCRIPTION( "My nice module" );
MODULE_SUPPORTED_DEVICE( "MyDevice" ); 

#define SUCCESS 0
#define DEVICE_NAME "MyDevice" /* Имя устройства */

// Поддерживаемые устройством операции
static int device_open( struct inode *, struct file * );
static int device_release( struct inode *, struct file * );
static ssize_t device_read( struct file *, char *, size_t, loff_t * );
static ssize_t device_write( struct file *, const char *, size_t, loff_t * );

// Глобальные переменные, объявлены как static, воизбежание конфликтов имен.
static int major_number; /* Старший номер устройства драйвера */
static int is_device_open = 0; 
static char text[ 16 ] = "Hi! I'm device!\n"; /* Текст, который будет при обращении к нашему устройству */
static char* text_ptr = text; /* Указатель на текущую позицию в тексте */

// обработчики операций на устройством
static struct file_operations fops =
 {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
 };

// Функция загрузки модуля
static int __init test_init( void )
{
 printk( KERN_ALERT "Driver loaded!\n" );

 // Регистрируем устройсво и получаем старший номер устройства
 major_number = register_chrdev( 0, DEVICE_NAME, &fops );

 if ( major_number < 0 )
 {
  printk( "Registering the character device failed with %d\n", major_number );
  return major_number;
 }

 // Сообщаем присвоенный нам старший номер устройства
 printk( "Module is loaded!\n" );

 printk( "Please, create a dev file with 'mknod /dev/MyDevice c %d 0'.\n", major_number );

 return SUCCESS;
}

// Функция выгрузки модуля
static void __exit test_exit( void )
{
 // Освобождаем устройство
 unregister_chrdev( major_number, DEVICE_NAME );

 printk( KERN_ALERT "Module is unloaded!\n" );
}

// Указываем наши функции загрузки и выгрузки
module_init( test_init );
module_exit( test_exit );

static int device_open( struct inode *inode, struct file *file )
{
 text_ptr = text;

 if ( is_device_open )
  return -EBUSY;

 is_device_open++;

 return SUCCESS;
}

static int device_release( struct inode *inode, struct file *file )
{
 is_device_open--;
 return SUCCESS;
}

static ssize_t

device_write( struct file *filp, const char *buff, size_t len, loff_t * off )
{
 printk( "Sorry, this operation isn't supported.\n" );
 return -EINVAL;
}

static ssize_t device_read( struct file *filp, 
       char *buffer, 
       size_t length, 
       loff_t * offset )
{
 int byte_read = 0;

 if ( *text_ptr == 0 )
  return 0;

 while ( length && *text_ptr )
 {
  put_user( *( text_ptr++ ), buffer++ );
  length--;
  byte_read++;
 }

 return byte_read;
}

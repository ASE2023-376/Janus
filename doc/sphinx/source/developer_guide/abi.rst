.. developer_guide abi

=============
ABI messaging
=============

Paparazzi use a custom publish/subscribe middleware to exchange data between software components named ``ABI``.

.. note:: This middleware was named *ABI* for *AirBorne Ivy*

General Idea
------------

* Give an easy way to allow software components to exchange data with minimum delay nor execution overhead.
* The components don't need to know each other, they only need the message format.
* Each subscriber set a callback function that will be called when new data are sent.

.. warning:: This middleware is **NOT** thread safe. Threads are not widely used in Paparazzi but remember this if you use them.


Message definition
------------------

The messages are described in ``conf/abi.xml`` analoguous to other messages in paparazzi. Names are unique, IDs starts from 0.
``"type"`` field should be a valid C type or pointer.
When adding a new message, make sure that the ``"id"`` is not used by any other message. After changing the XML file, you need to
run ``make clean`` and ``make`` in the ``paparazzi`` folder to generate the corresponding .c and .h files (for more information
on the code generation have a look at the `In depth`_ section).

.. code-block:: xml

    <class name="airborne">
     <message name="DATA" id="0">
      <field name="a" type="float"/>
      <field name="b" type="struct bla"/>
      <field name="c" type="struct foo *"/>
     </message>
     ...
    </class>

The same message can be used by different modules. For example, opticflow based modules, GPS modules, airspeed sensors, etc.
are all capable of sending a ``VELOCITY_ESTIMATE``, and it's often important to be able to identify the source of the message.
The ``SENDER_ID`` are defined in ``sw/airborne/modules/core/abi_sender_ids.h``. Sender IDs should be unique **within a
message type**.

.. note:: Sender ID values should be unique within a message type. They are "children" of their message ID.

Your sender ID identifier should be constructed as the concatenation of the name of the message and the name of your module, suffixed with ``_ID``.
For example, a good sender ID for a module ``toto`` sending the message ``DATA`` may be ``DATA_TOTO_ID``

.. code-block:: C

    /*
     * IDs of DATA message (message 0)
     */
    #ifndef DATA_TOTO_ID
    #def DATA_TOTO_ID 1
    #endif

.. warning::

    The values 0 and 255 are reserved for ``ABI_DISABLE`` and ``ABI_BROADCAST`` and thus shall not be used.

    * ``ABI_BROADCAST`` (255) is used to receive messages from all senders.
    * ``ABI_DISABLE`` (0) disable the callback (it will never be called).


Airborne code for subscriber
----------------------------

To receive a message in your module, include header and declare an ``abi_event`` as a global ``static`` variable (but **not** ``const``).
Write the callback function with the proper prototype, matching the message. This means that the arguments of the callback
function should be ``sender_id``, followed by all the fields defined in the message. Each argument type needs to match exactly!

.. code-block:: C

    #include "modules/core/abi.h"

    static abi_event ev;

    void data_cb(uint8_t sender_id, float a, struct bla b, struct foo * c) {
     // do something here
    }

.. warning:: Make sure the callback doesn't take too much time (better to process the received data elsewhere), to avoid
  delaying the execution of the main loop.

In the initialization function (or later) call the binding function for the message you want to receive.

.. code-block:: C

    AbiBindMsgDATA(ABI_BROADCAST, &ev, data_cb);

The first parameter is the sender ID you want to receive the message from.
Senders IDs can be found in the file ``sw/airborne/modules/core/abi_sender_ids.h``

.. warning::

    The values 0 and 255 are reserved for ``ABI_DISABLE`` and ``ABI_BROADCAST`` and thus shall not be used.

    You must also avoid using an ID already used to send the same message type.

The second parameter is a pointer to the global ``abi_event`` you declared. This variable **can't** be reused for another bind.
You must declare one abi_event per bind.

The last parameter is your callback.

.. note:: When creating your module with the Module Creator (in the Paparazzi Center : Tools -> Module Creator), you can add subcriptions to ABI messages.


Airborne code for publisher
---------------------------

Include header, then call the send function with the appropriate parameters

.. code-block:: C

    #include "modules/core/abi.h"

    float var = 2.;
    struct bla s;
    struct foo f;
    AbiSendMsgDATA(SENDER_ID, var, s, &f);

Replace ``SENDER_ID`` by your sender ID defined in ``sw/airborne/modules/core/abi_sender_ids.h``.


Creating a new message
------------------------

Creating a new message requires multiple files to be edited. Here are all the steps:

#. In ``conf/abi.xml`` find an unused message id and define a new message block as described in `Message definition`_

#. In ``sw/airborne/modules/core/abi_sender_ids.h`` define a ``SENDER_ID`` for your message as described in `Message definition`_

#. Run ``make clean`` and ``make`` in the ``paparazzi`` folder

#. In your module .c file where the new message should be sent, include the header ``"modules/core/abi.h"``, and call
   the function ``AbiSendMsg<YOUR_MESSAGE_NAME>(<YOUR_SENDER_ID>, ...)`` with the appropriate function parameters corresponding
   to the message fields, as shown in `Airborne code for publisher`_.

#. To receive this message in another module .c file, follow the steps described in `Airborne code for subscriber`_

.. note::
  It is also possible to automatically generate the necessary ABI bindings and callbacks when creating a new module via the
  GUI tool Module Creator (accessible from the Paparazzi Center in the menu Tools -> Module Creator).


Code generation
---------------

The generated code will be in ``var/include/abi_messages.h`` and include some structure definition from ``sw/airborne/modules/core/abi_common.h``
(``sw/airborne/modules/core/abi.h`` is a convenience header that only includes ``var/include/abi_messages.h``).

Bind and Send functions are generated, as well as callback type definition. A linked list is used to store the bound
callbacks for each message. The head of the list is in an array to allow a fast access.

The code is generated by ``/sw/tools/generators/gen_abi.xml``.

In depth
--------

Generated code
______________

Here is the code of ``sw/airborne/modules/core/abi_common.h``:

.. code-block:: C

    /* Include here headers with structure definition you may want to use with ABI
     * Ex: '#include "modules/gps/gps.h"' in order to use the GpsState structure
     */
    #include "modules/gps/gps.h"

    #include "modules/core/abi_sender_ids.h"

    #ifdef ABI_C
    #define ABI_EXTERN
    #else
    #define ABI_EXTERN extern
    #endif

    /** Generic callback definition */
    typedef void (*abi_callback)(void);

    // ABI Broadcast address.
    #define ABI_BROADCAST 255

    // ABI disable address
    #define ABI_DISABLE 0

    /** Event structure to store callbacks in a linked list */
    struct abi_struct {
      uint8_t id;
      abi_callback cb;
      struct abi_struct *next;
    };
    typedef struct abi_struct abi_event;

    /** Macros for linked list */
    #define ABI_FOREACH(head,el) for(el=head; el; el=el->next)
    #define ABI_PREPEND(head,add) { (add)->next = head; head = add; }




The generated code in ``var/include/abi_messages.h`` for the message defined above will look like this:

.. code-block:: C

    // Code generated in var/include/abi_messages.h
    #include "modules/core/abi_common.h
    // Message IDs
    #define ABI_DATA_ID 0
    
    // Array and linked list
    #define ABI_MESSAGE_NB <highest id of the messages in airborne class + 1>
    ABI_EXTERN abi_event* abi_queues[ABI_MESSAGE_NB]; // Magic trick to avoid generating .c file

    // Callbacks
    typedef void (*abi_callbackDATA)(uint8_t sender_id, float a, struct bla b, struct foo * c); // Specific callback for DATA message (arguments are const to prevent modifying them)

    // Bind and Send for each messages
    static inline void AbiBindMsgDATA(uint8_t sender_id, abi_event * ev, abi_callbackDATA cb) {
      if (abi_queues[ABI_BARO_ABS_ID] == ev) return;
      ev->id = sender_id;
      ev->cb = (abi_callback)cb;
      ABI_PREPEND(abi_queues[ABI_BARO_ABS_ID],ev);
    }

    static inline void AbiSendMsgDATA(uint8_t sender_id, float a, struct bla b, struct foo * c) {
     // Call all callback functions
     abi_event* e;
     ABI_FOREACH(abi_queues[ABI_DATA_ID],e) {
      if (e->id == ABI_BROADCAST || e->id == sender_id) { // call function only if selected source or broadcast address
       abi_callbackDATA cb = (abi_callbackDATA)(e->cb); // C black magic
       cb(sender_id, a, b, c);
      }
     }
    }

Using ABI in custom code
________________________


If you want to use ABI outside of one of the Paparazzi firmwares (``rotorcraft``, ``rover`` or ``fixedwing``), this code snippet should be written and called only once in a .c file (most probably your ``main.c``) to enable ABI:

.. code-block:: C

    #define ABI_C 1
    #include "modules/core/abi.h"


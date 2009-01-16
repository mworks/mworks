// TODO: improvements in key event handling are required
#ifndef KEY_TRIGGER_H
#define KEY_TRIGGER_H

#include "GenericVariable.h"
/**
 * KeyTrigger.h
 *
 * Description: Encapsulates a key combination and triggers an event when the
 * combination is pressed by user.
 *
 * History:
 * ??/??/?? ??? Created.
 * 05/17/04 PAJ Added inspection methods.
 */
namespace mw {
enum KeyModifier{ NO_MODIFIER, OPTION_KEY, COMMAND_KEY, ALT_KEY, CONTROL_KEY, SHIFT_KEY};


class KeyTrigger{

	protected:
	
		char key;
		KeyModifier modifier;
		Variable *togglevar;
		Variable *active;
		
		bool setadditionalvalue;
		Variable *radiovar;
		Data value; 			// not implemented as a template for now to reduce complexity
		
	/*	char getMask(int index){
		
			switch(index){
				
				case ONE_KEY:
					return (char)1;
				case OPTION_KEY:
					return 0x0002;
				case COMMAND_KEY:
					return 0x0001;
				case SHIFT_KEY:
					return 0x0004;
				case CTRL_KEY:
					return 0x0002;
			}
		}
		*/
		
	public:
	
		KeyTrigger(char _key, Variable *_toggle, Variable *_active){
			key = _key;
			modifier = NO_MODIFIER;
			togglevar = _toggle;
			active = _active;
			
			setadditionalvalue = false;
		}
		
		KeyTrigger(KeyModifier _modifier, char _key, Variable *_toggle, Variable *_active){
			
			key = _key;
			modifier = _modifier;
			togglevar = _toggle;
			active = _active;
			
			setadditionalvalue = false;
		}
		
		KeyTrigger(char _code, Variable *_toggle, Variable *_radiovar, Data _val, Variable *_active){
		
			key = _code;
			modifier = NO_MODIFIER;
			togglevar = _toggle;
			active = _active;
			
			radiovar = _radiovar;
			value = _val;
			
			setadditionalvalue = true;
		}
		
		KeyTrigger(KeyModifier _modifier, char _key, Variable *_toggle, Variable *_radiovar, Data _val, Variable *_active){
		
			key = _key;
			modifier = _modifier;
			togglevar = _toggle;
			active = _active;
			
			radiovar = _radiovar;
			value = _val;
			
			setadditionalvalue = true;
		}
        
		void trigger();  // called from somewhere else when the key is pressed

        char getKey() { return key; }
        KeyModifier getKeyModifier() { return modifier; }
        bool isAdditionalValueSet() { return setadditionalvalue; }
        bool isActive();
        Variable * getToggleVar() { return togglevar; }
        //additional data items
        Variable * getRadioVar() { return radiovar; }
        Data * getValue() { return &value; }


/*
		void evaluate(EventRecord *pEvent){
		
			// TODO: this needs to be improved in a number of ways
			if(	(modifier == NO_MODIFIER) ||
				((pEvent->modifiers & cmdKey) && (modifier == COMMAND_KEY)) ||
				((pEvent->modifiers & optionKey) && (modifier == OPTION_KEY))){
				
				char thechar = pEvent->message & charCodeMask;
				
				if((pEvent->message & charCodeMask) & key){
					trigger();
				}
			}
		}
	*/				
};
}

#endif
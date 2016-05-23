# Script-and-Go

> A chapter of featherless' [digital creative history](https://github.com/featherless/digital-creative-history).

The Script-and-Go scripting language was built to allow people to build OpenGL applications without a compiler.

## Example script

    /*  Script-and-Go code by Jeff Verkoeyen,
    	created 2003	*/
    
    VARIABLES;
    	@int integer;
    	@float integer2;
    	@char testing;
    
    	testing="hey";
    
    	integer2=34;
    
    BEGIN;
    	integer+234;
    	DrawMode("front","lines");
    	Reset;							// Equivalent to LoadIdentity
    	DisableTextures;				// Disables the textures
    	Move(0,0,-100);					// Moves the line back on the z-plane
    	SetColor(1.0f,0.5f,0.25f,0.0f);	// Sets the color to orange
    	DrawLine(0,0,0,38,20,0);		// Draws the line
    	DrawLine(30,20,0,-10,-30,40);	// Draws another line, but closer
    
    	Reset;
    	Move(0,0,-100);
    	SetColor(1.0f,1.0f,0.0f,0.5f);
    	Rotate(integer,1.0f,0.0f,0.0f);
    	Move(10,20,0);
    	Box(-10,10,0,10,10,0,10,-10,0,-10,-10,0);
    END;

# License

All source code is licensed Apache 2.0.

> A chapter of featherless' [digital creative history](https://github.com/featherless/digital-creative-history).

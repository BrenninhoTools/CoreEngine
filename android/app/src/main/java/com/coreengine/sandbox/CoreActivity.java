package com.coreengine.sandbox;

import org.libsdl.app.SDLActivity;

public class CoreActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "coreengine_sandbox"
        };
    }
}

import { Stack } from "expo-router";
import { useCallback, useEffect, useState } from 'react';
import { useFonts } from 'expo-font';
import * as SplashScreen from 'expo-splash-screen';
import { View, Text, ActivityIndicator } from 'react-native';
import { COLORS } from '../constants';

// Prevent the splash screen from auto-hiding when the app first loads
// This needs to be called outside of any component
SplashScreen.preventAutoHideAsync().catch((error) => {
  console.warn("Error preventing splash screen from auto-hiding:", error);
});

const Layout = () => {
  const [appIsReady, setAppIsReady] = useState(false);
  const [error, setError] = useState<string | null>(null);
  
  // Load custom fonts
  const [fontsLoaded, fontError] = useFonts({
    DMBold: require('../assets/fonts/DMSans-Bold.ttf'),
    DMMedium: require('../assets/fonts/DMSans-Medium.ttf'),
    DMRegular: require('../assets/fonts/DMSans-Regular.ttf'),
    PokemonFont: require('../assets/fonts/pokemon_Gen1.ttf'),
  });

  // Handle font loading and app initialization
  useEffect(() => {
    async function prepare() {
      try {
        console.log("App initializing and loading fonts...");
        
        // Wait for fonts to load
        if (fontsLoaded) {
          console.log("Fonts loaded successfully");
          
          // Perform any other initialization tasks here
          // For example: load cached data, initialize services, etc.
          
          // Artificial delay to ensure fonts are properly applied
          // Remove this in production if not needed
          await new Promise(resolve => setTimeout(resolve, 500));
          
          // Mark the app as ready
          setAppIsReady(true);
        }
        
        if (fontError) {
          console.error("Error loading fonts:", fontError);
          setError("Failed to load application fonts");
        }
      } catch (e) {
        console.warn("Error during app initialization:", e);
        setError("Failed to initialize the application");
      }
    }

    prepare();
  }, [fontsLoaded, fontError]);

  // Handle hiding the splash screen once the app is ready
  const onLayoutRootView = useCallback(async () => {
    if (appIsReady) {
      console.log("App is ready, hiding splash screen");
      try {
        // Hide the splash screen
        await SplashScreen.hideAsync();
        console.log("Splash screen hidden successfully");
      } catch (e) {
        console.warn("Error hiding splash screen:", e);
      }
    }
  }, [appIsReady]);

  // Show an error screen if something went wrong
  if (error) {
    return (
      <View style={{ flex: 1, justifyContent: 'center', alignItems: 'center', backgroundColor: '#fff' }}>
        <Text style={{ fontFamily: 'DMBold', fontSize: 18, color: 'red', textAlign: 'center', padding: 20 }}>
          {error}
        </Text>
        <Text style={{ fontFamily: 'DMRegular', fontSize: 16, color: '#333', textAlign: 'center', marginTop: 10 }}>
          Please restart the application.
        </Text>
      </View>
    );
  }
  
  // Show a loading screen if the app is not ready yet
  if (!appIsReady) {
    return (
      <View style={{ flex: 1, justifyContent: 'center', alignItems: 'center', backgroundColor: '#fff' }}>
        <ActivityIndicator size="large" color={COLORS.primary} />
        <Text style={{ marginTop: 20, fontFamily: 'DMRegular', fontSize: 16 }}>
          Loading...
        </Text>
      </View>
    );
  }

  // Main app layout with proper onLayout handler to hide splash screen
  return (
    <View style={{ flex: 1 }} onLayout={onLayoutRootView}>
      <Stack 
        screenOptions={{
          headerShown: true,
          headerStyle: {
            backgroundColor: 'transparent',
          },
          headerShadowVisible: false,
          headerTransparent: true,
          animation: 'slide_from_right',
        }}
      >
        <Stack.Screen 
          name="index" 
          options={{ 
            headerTitle: "",
          }}
        />
        <Stack.Screen 
          name="stats/[name]" 
          options={{ 
            headerTitle: "",
            presentation: "card",
          }}
        />
      </Stack>
    </View>
  );
}

export default Layout;

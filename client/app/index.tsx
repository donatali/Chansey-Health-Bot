import { useState } from 'react';
import { 
  View, 
  Text, 
  ScrollView, 
  SafeAreaView,
  ImageBackground,
  StyleSheet 
} from 'react-native';
import { Stack, useRouter } from 'expo-router';
import { useFonts } from 'expo-font';
import allNames from "../constants/getAllNames";

import { COLORS, icons, images, SIZES } from '../constants';

import {
  RecentUsers, PopularUsers, ScreenHeaderBtn, Welcome
} from '../components';

const Home = () => {
  const router = useRouter();
  const [searchTerm, setSearchTerm] = useState('');

  const [fontsLoaded] = useFonts({
    PokemonFont: require('../assets/fonts/pokemon_Gen1.ttf'),
  });

  if (!fontsLoaded) return null;

  const handleSearchClick = () => {
    const inputName = searchTerm.trim().toLowerCase();
    if (allNames.includes(inputName)) {
      router.push(`/stats/${encodeURIComponent(searchTerm.trim())}`);
    } else {
      alert("Trainer not found! Please check the name or use the tabs.");
    }
  };

  return (
    <ImageBackground
      source={require('../assets/backgrounds/pokemon_gen2.jpg')}
      style={styles.background}
      resizeMode="cover"
    >
      <SafeAreaView style={styles.safeArea}>
        <Stack.Screen
          options={{
            headerStyle: { backgroundColor: 'transparent' },
            headerShadowVisible: false,
            headerLeft: () => (
              <ScreenHeaderBtn 
                iconUrl={icons.menu} 
                dimension="60%"
                handlePress={() => {}}
              />
            ),
            headerRight: () => (
              <ScreenHeaderBtn 
                iconUrl={images.profile} 
                dimension="100%"
                handlePress={() => {}}
              />
            ),
            headerTitle: '',
          }}
        />

        <ScrollView showsVerticalScrollIndicator={false}>
          <View style={styles.container}>
            <View style={styles.messageBox}>
              <Text style={styles.messageText}>Welcome back, Trainer!</Text>
            </View>

            <Welcome 
              searchTerm={searchTerm}
              setSearchTerm={setSearchTerm}
              handleClick={handleSearchClick}
            />
            <PopularUsers />
            <RecentUsers />
          </View>
        </ScrollView>
      </SafeAreaView>
    </ImageBackground>
  );
};

const styles = StyleSheet.create({
  background: {
    flex: 1,
    width: '100%',
    height: '100%',
  },
  safeArea: {
    flex: 1,
  },
  container: {
    flex: 1,
    padding: SIZES.medium,
  },
  messageBox: {
    backgroundColor: 'white',
    borderColor: 'black',
    borderWidth: 2,
    borderRadius: 8,
    padding: 12,
    marginBottom: 20,
  },
  messageText: {
    fontFamily: 'PokemonFont',
    fontSize: 18,
    color: 'black',
  },
});

export default Home;
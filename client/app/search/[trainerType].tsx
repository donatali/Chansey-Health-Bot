import { useLocalSearchParams } from "expo-router";
import { View, Text, FlatList, Image, StyleSheet, ImageBackground, TouchableOpacity} from 'react-native';
import { useFonts } from "expo-font";
import { useRouter } from "expo-router";
import { Stack } from "expo-router";
import { Ionicons } from "@expo/vector-icons";

import mockData from "../../constants/trainerData";

const backgrounds = {
    "Student": require("../../assets/backgrounds/student_bg.jpg"),
    "IA/GSI": require("../../assets/backgrounds/ia_gsi_bg1.jpg"),
    "Professor/Mentor": require("../../assets/backgrounds/prof_mentor_bg.jpg"),
};

const TrainerList = () => {
    const router = useRouter();
    const {trainerType } = useLocalSearchParams();
    
    const [fontsLoaded] = useFonts({
        PokemonFont: require("../../assets/fonts/pokemon_Gen1.ttf"),
    });
    
    if(!fontsLoaded) return null;

    const data = mockData[trainerType] || [];
    const background = backgrounds[trainerType];

    return (
        <ImageBackground
            source={background}
            style={styles.background}
            resizeMode="cover">
            <Stack.Screen
                options={{
                    headerTitle: "",
                    headerTransparent: true,
                    headerLeft: () => (
                        <TouchableOpacity onPress={() => router.back()} style={{ marginLeft: 10 }}>
                          <Ionicons name="home-outline" size={24} color="black" />
                        </TouchableOpacity>
                      ),
                    headerShadowVisible: false,
                }}
            />
            <View style={styles.container}>
                <View style={styles.titleBox}>
                    <Text style={styles.title}>{trainerType}</Text>
                </View>
                <FlatList
                  data={data}
                  keyExtractor={(item, index) => item.name + index}
                  renderItem={({ item }) => (
                      <TouchableOpacity
                      onPress={() => router.push(`/stats/${item.name}`)}
                          style={styles.listItem}
                      >                        
                          <Image
                              source={require("../../assets/buttons/pokeball_button.png")}
                              style={styles.homeIcon}
                              resizeMode="contain"
                          />
                          <Image source={item.image} style={styles.avatar} />
                          <Text style={styles.name}>{item.name}</Text>
                      </TouchableOpacity>
                  )} 
                />
            </View>
        </ImageBackground>
    );
};

    const styles = StyleSheet.create({
        background: {
            flex:1,
            width: "100%",
            height: "100%",
        },
        overlay: {
            ...StyleSheet.absoluteFillObject,
            backgroundColor: "rgba(255,255,255,0.7",
        },
        container: {
          flex: 1,
          //backgroundColor: "#fff",
          paddingTop: 200,
          paddingHorizontal: 20,
        },
        homeButton: {
            position: "absolute",
            bottom: 30,
            alignSelf: "center",
            backgroundColor: "#f2f2f2",
            padding: 15,
            borderRadius: 50,
            shadowColor: "#000",
            shadowOffset: { width: 0, height: 8 },
            shadowOpacity: 0.3,
            shadowRadius: 10,
            elevation: 12,
          },
          homeIcon: {
            width: 50,
            height: 50,
          },
        title: {
          fontSize: 28,
          fontFamily: "PokemonFont",
          color: "black",
          //fontWeight: "bold",
          marginBottom: 20,
        },
        titleBox: {
            alignSelf: "flex-start",
            backgroundColor: "white",
            borderRadius: 12,
            borderWidth: 2,
            borderColor: "black",
            paddingVertical: 8,
            paddingHorizontal: 16,
            marginBottom: 20,
          },
        listItem: {
          flexDirection: "row",
          alignItems: "center",
          paddingVertical: 10,
          paddingHorizontal: 10,
          backgroundColor: "white",
          borderRadius: 10,
          borderWidth: 1.5,
          borderColor: "black",
          marginBottom: 10,
        },
        avatar: {
          width: 60,
          height: 60,
          borderRadius: 30,
          marginRight: 15,
        },
        name: {
          fontSize: 18,
          fontFamily: "PokemonFont",
          color: "black",
        },
      });
      
      export default TrainerList;

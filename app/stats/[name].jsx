
import { Stack, useLocalSearchParams, useRouter } from "expo-router";
import { 
  View, 
  Text, 
  StyleSheet, 
  ScrollView, 
  ActivityIndicator,
  ImageBackground,
  TouchableOpacity,
} from "react-native";
import { useFonts } from "expo-font";
import { Ionicons } from "@expo/vector-icons";
import useFetch from "../../hook/useFetch";

const roundToSignicantFigure = (num, sigFigs = 1) => {
  if (num === 0) return 0;
  const multiplier = Math.pow(10, Math.floor(Math.log10(Math.abs(num))) + 1 - sigFigs);
  return Math.round(num * multiplier) / multiplier;
};

const StatsPage = () => {
  const { name } = useLocalSearchParams();
  const router = useRouter();

  const[fontsLoaded] = useFonts({
    PokemonFont: require("../../assets/fonts/pokemon_Gen1.ttf"),
  })

  const { data, isLoading, error } = useFetch("receive", "GET", null, name);

  if(!fontsLoaded) return null;

  const filtered = (data?.events || []).filter((event) => event.name === name);
  const numericTemps = filtered
    .map((e) => parseFloat(e.temperature))
    .filter(temp => temp >= 85 && temp <= 100.3)
    .map(temp => roundToSignicantFigure(temp, 1));
  const numericHRs = filtered.map(e => parseFloat(e.heart_rate));
  
  const avgTemp = numericTemps.length > 0
    ? roundToSignicantFigure(
        numericTemps.reduce((a, b) => a + b, 0) / numericTemps.length,
        1
      ).toFixed(1)
    : "N/A";

  const avgHR = numericHRs.length > 0
    ? (numericHRs.reduce((a, b) => a + b, 0) / numericHRs.length).toFixed(0)
    : "N/A";

  const background = require("../../assets/backgrounds/healing_1.png");

  return (
    <ImageBackground 
    source={background} 
    style={styles.background} 
    resizeMode="cover"
    >
      <Stack.Screen
        options={{
          headerTitle: "",
          headerTransparent: true,
          headerShadowVisible: false,
          headerLeft: () => (
            <TouchableOpacity onPress={() => router.back()} style={{ marginLeft: 10 }}>
              <Ionicons name="arrow-back-outline" size={28} color="black" />
            </TouchableOpacity>
          ),
        }}
      />

      <ScrollView contentContainerStyle={styles.container}>
      <View style={styles.titleBox}>
        <Text style={styles.name}>{name}</Text>
      </View>
      <View style={styles.avgContainer}>
        <View style={styles.avgBox}>
          <Text style={styles.avgLabel}>Avg HR</Text>
          <Text style={styles.avgValue}>{avgHR} bpm</Text>
        </View>
        <View style={styles.avgBox}>
          <Text style={styles.avgLabel}>Avg Temp</Text>
          <Text style={styles.avgValue}>{avgTemp}°F</Text>
        </View>
      </View>

        {isLoading ? (
          <ActivityIndicator size="large" style={{ marginTop: 40 }} />
        ) : error ? (
          <Text style={styles.noData}>Error fetching stats.</Text>
        ) : filtered.length === 0 ? (
          <Text style={styles.noData}>No data available for {name}</Text>
        ) : (
            
        //round temperature
        filtered.map((event, index) => {
            const roundedTemp = roundToSignicantFigure(parseFloat(event.temperature), 1);

            return(
              <View key={index} style={styles.eventCard}>
                <View style={styles.statBox}><Text style={styles.stat}>🫀 Heart Rate: {event.heart_rate}</Text></View>
                <View style={styles.statBox}>
                  <Text style={[styles.stat, roundedTemp >= 100.4 && styles.highTemp]}>
                    🌡️ Temperature: {roundedTemp}
                  </Text>
                </View>

              <View style={styles.statBox}>
                <Text style={styles.stat}>💊 Pill Dispensed: {event.pill_dispensed}</Text></View>
              <Text style={styles.timestamp}>⏱️ {event.timestamp}</Text>
            </View>
          );
        })
      )}
      </ScrollView>
    </ImageBackground>
  );
};

const styles = StyleSheet.create({
  background: {
    flex: 1,
    width: "100%",
    height: "100%",
  },
  container: {
    padding: 20,
    paddingTop: 140,
  },
  name: {
    fontSize: 36,
    fontFamily: "PokemonFont",
    marginBottom: 20,
    textAlign: "center",
    color: "black",
  },
  highTemp: {
    color: "red",
  },
  eventCard: {
    flexDirection: "column",
    backgroundColor: "white",
    borderRadius: 10,
    borderWidth: 1.5,
    borderColor: "black",
    padding: 15,
    marginBottom: 15,
  },
  stat: {
    fontSize: 16,
    fontFamily: "PokemonFont",
    color: "black",
  },
  statBox: {
    backgroundColor: "white",
    borderWidth: 1.5,
    borderColor: "black",
    borderRadius: 8,
    paddingVertical: 4,
    paddingHorizontal: 10,
    marginBottom: 6,
  },
  timestamp: {
    fontSize: 14,
    fontFamily: "PokemonFont",
    color: "gray",
    marginTop: 5,
  },
  noData: {
    fontSize: 18,
    fontFamily: "PokemonFont",
    textAlign: "center",
    marginTop: 40,
    color: "black",
  },
  titleBox: {
    alignSelf: "center",
    backgroundColor: "white",
    borderRadius: 12,
    borderWidth: 2,
    borderColor: "black",
    paddingVertical: 8,
    paddingHorizontal: 16,
    marginBottom: 20,
  },
  avgContainer: {
    flexDirection: "row",
    justifyContent: "center",
    gap: 20,
    marginBottom: 20,
  },
  avgBox: {
    backgroundColor: "white",
    borderColor: "black",
    borderWidth: 2,
    borderRadius: 12,
    paddingVertical: 8,
    paddingHorizontal: 12,
    alignItems: "center",
  },
  avgLabel: {
    fontSize: 16,
    fontFamily: "PokemonFont",
    color: "gray",
  },
  avgValue: {
    fontSize: 20,
    fontFamily: "PokemonFont",
    color: "black",
  },
});

export default StatsPage;

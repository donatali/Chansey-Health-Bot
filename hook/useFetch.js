import { useState, useEffect } from "react";
//import { io } from "socket.io-client";
import axios from "axios";
import AsyncStorage from "@react-native-async-storage/async-storage";


const BASE_URL = "http://localhost:3000";
//const cacheKey = `cache_recieve`;

const useFetch = (endpoint, method = "GET", body = null, userId = null) => {
  const [data, setData] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);
  const [lastTimestamp, setLastTimestamp] = useState(null);
  const intervalRef = useRef(null);

  const cacheKey = `cache_${endpoint}${userId ? `_${userId}` : ""}`;



  const fetchData = async () => {
    setIsLoading(true);

    try {
      const url = `${BASE_URL}/${endpoint}${userId ? `?user_id=${userId}` : ""}`;
      const response = await axios({ method, url, data: body });

      const newData = response.data?.events || response.data;
      if(newData && newData.length > 0) {
        //const latestEvent = newData[newData.length - 1];
        const filtered = newData.filter(item =>
          !lastTimestamp || new Date(item.timestamp) > new Date(lastTimestamp)
        );
        //if(latestEvent.timestamp !== lastTimestamp) {
        if (filtered.length > 0) {
          const updatedData = [...data, ...filtered];
          const latest = filtered[filtered.length - 1].timestamp;

          setData(updatedData);
          setLastTimestamp(latest);
          await AsyncStorage.setItem(cacheKey, JSON.stringify(updatedData));
          setError(null);
        }
      }
    } catch (err) {
      console.warn(`Fetch error: ${err}. Trying cache....`);
      setError(err);
    
      try {
        const cacheData = await AsyncStorage.getItem(cacheKey);
        if (cacheData) {
          const parsed = JSON.parse(cacheData);
          setData(parse);
          if (parsed.length > 0) {
            setLastTimestamp(parsed[parsed.length - 1].timestamp);
          }
        }
      } catch (cacheErr) {
        console.error("Error landing cached data:", cacheErr);
      }
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {

    const loadInitial= async () => {
      const cacheData = await AsyncStorage.getItem(cacheKey);
      if (cacheData) {
        const parsed = JSON.parse(cacheData);
        setData(parsed);
        if (parsed.length > 0) {
          setLastTimestamp(parsed[parsed.length - 1].timestamp);
        }
      }
        fetchData();
    };

    loadInitial();

    if(method == "GET" && !intervalRef.current) {
      intervalRef.current = setInterval(() => {
        fetchData();
      }, 30000);
    }
  
    return () => clearInterval(intervalRef.current);
  }, [endpoint, method, userId]);

  const refetch = () => fetchData();

  return { data, isLoading, error, refetch };
};

export default useFetch;

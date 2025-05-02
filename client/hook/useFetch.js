import { useState, useEffect } from "react";
import axios from "axios";

const BASE_URL = "http://localhost:3000";
//const BASE_URL = "http://35.0.28.75:3000";

const useFetch = (endpoint, method = "GET", body = null, userId = null) => {
  const [data, setData] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);

  const fetchData = async () => {
    setIsLoading(true);
    try {
      const url = `${BASE_URL}/${endpoint}${userId ? `?user_id=${userId}` : ""}`;
      console.log(`[useFetch] Fetching data from: ${url}`);
      
      const config = {
        method: method.toLowerCase(),
        url: url,
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json'
        }
      };
      
      if (['post', 'put', 'patch'].includes(method.toLowerCase()) && body) {
        config.data = body;
      }
      
      const response = await axios(config);
      console.log(`[useFetch] Received response status: ${response.status}`);
      
      if (response.data && 'events' in response.data) {
        console.log(`[useFetch] Found ${response.data.events.length} events in response`);
        setData(response.data.events);
      } else {
        console.log(`[useFetch] No 'events' key found in response, using entire response`);
        setData(Array.isArray(response.data) ? response.data : []);
      }
      
      setError(null);
    } catch (err) {
      console.error(`[useFetch] Error:`, err);
      setError(err);
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, [endpoint, userId]);

  const refetch = () => {
    fetchData();
  };

  return { data, isLoading, error, refetch };
};

export default useFetch;

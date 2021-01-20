import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class ConvertRouteFileUrban {
	
	public static String Datum = "Datum";
	public static String Uhrzeit = "Uhrzeit";
	
	public static String TAB_DELIMITER = "\t";
	public static String COMMA_DELIMITER = ",";
	public static String SEMICOLON_DELIMITER = ";";
	public static String USE_DELIMITER = SEMICOLON_DELIMITER;
	
	public static String DK51 = "D1Z"; //nr1
	public static String DK52 = "D2Z"; //nr2
	public static String DK53 = "D3Z"; //nr3
	public static String DK54 = "D4Z"; //nr4
	public static String DK71 = "D5Z"; //nr5
	public static String DK72 = "D6Z"; //nr6
	public static String DK91 = "D7Z"; //nr7
	public static String DK92 = "D8Z"; //nr8
	public static String DK93 = "D9Z"; //nr9
	public static String DK94 = "D10Z"; //nr10
	
	public static void main(String[] args) throws IOException {
		String inputFilePath = "/home/longpham211/Downloads/zst6826/export.csv";
		String outputFilePath = "/home/longpham211/Downloads/zst6826/routeUrban.txt";
		
		
		int Datum_index = 0;
		int Uhrzeit_index = 0;
		
		int DK51_index = 0;
		int DK52_index = 0;
		int DK53_index = 0;
		int DK54_index = 0;
		int DK71_index = 0;
		int DK72_index = 0;
		int DK91_index = 0;
		int DK92_index = 0;
		int DK93_index = 0;
		int DK94_index = 0;
		
	    File outputFile = new File(outputFilePath);
	    Writer writer = new FileWriter(outputFile, true);
	    
		List<List<String>> records = new ArrayList<>();
		
		BufferedReader br = new BufferedReader(new FileReader(inputFilePath));
	    String line;
	    line = br.readLine(); // read the first line
	    
        String[] values = line.split(USE_DELIMITER);
        records.add(Arrays.asList(values)); // should be as records(0)
		
        for(int i = 0 ; i < values.length ; i++) {
        	if(values[i].equals(Datum))
        		Datum_index = i;
        	else if(values[i].equals(Uhrzeit))
        		Uhrzeit_index = i;
        	else if(values[i].equals(Uhrzeit))
        		Uhrzeit_index = i;
        	else if(values[i].equals(DK51))
        		DK51_index = i;
        	else if(values[i].equals(DK52))
        		DK52_index = i;
        	else if(values[i].equals(DK53))
        		DK53_index = i;
        	else if(values[i].equals(DK54))
        		DK54_index = i;
        	else if(values[i].equals(DK71))
        		DK71_index = i;
        	else if(values[i].equals(DK72))
        		DK72_index = i;
        	else if(values[i].equals(DK91))
        		DK91_index = i;
        	else if(values[i].equals(DK92))
        		DK92_index = i;
        	else if(values[i].equals(DK93))
        		DK93_index = i;
        	else if(values[i].equals(DK94))
        		DK94_index = i;
        }

		String vehicleType = "pkw_ohne_anhaenger";
		int routes = 3;
		int days = 3;
		int[][] accumulateVechiclesVector = new int[routes][days * 24];
		int route5_index = 0;
		int route7_index = 1;
		int route9_index = 2;
		
	    while ((line = br.readLine()) != null) {
	        values = line.split(USE_DELIMITER);
//	        int baseHourValue = 0;
//	        if(!(values[Datum_index].equalsIgnoreCase("06.06.2020") || values[Datum_index].equalsIgnoreCase("180805") || values[Datum_index].equalsIgnoreCase("180806")))
//	        	continue;
//	        else if (values[Datum_index].equalsIgnoreCase("07.06.2020")) {
//	        	baseHourValue = 24;
//	        }
//	        else if (values[Datum_index].equalsIgnoreCase("180806")) {
//	        	baseHourValue = 48;
//	        }
	    
	        int route5, route7, route9;
	        route5 = route7 = route9 = 0;
	        route5 += getMaxNumberBetweenTwo(Integer.valueOf(values[DK51_index]), Integer.valueOf(values[DK53_index]));
	        route5 += getMaxNumberBetweenTwo(Integer.valueOf(values[DK52_index]), Integer.valueOf(values[DK54_index]));
	        
	        route7 += getMaxNumberBetweenTwo(Integer.valueOf(values[DK71_index]), Integer.valueOf(values[DK72_index]));
	        
	        route9 += getMaxNumberBetweenTwo(Integer.valueOf(values[DK91_index]), Integer.valueOf(values[DK93_index]));
	        route9 += getMaxNumberBetweenTwo(Integer.valueOf(values[DK92_index]), Integer.valueOf(values[DK94_index]));
	    
	        int dayValue = Integer.valueOf(values[Datum_index].substring(0, 2));
	        int hourValue = Integer.valueOf(values[Uhrzeit_index].substring(0, 2));
	        
	        accumulateVechiclesVector[route5_index][hourValue + (dayValue - 6) * 24] += route5;
	        accumulateVechiclesVector[route7_index][hourValue + (dayValue - 6) * 24] += route7;
	        accumulateVechiclesVector[route9_index][hourValue + (dayValue - 6) * 24] += route9;
	    }
	    
	    
	    for (int i = 0; i < accumulateVechiclesVector[0].length; i++) {
	    	double beginValue = i * 3600;
	    	double endValue = (i + 1) * 3600;
	    	String output;

	    	beginValue += 0.07;
	    	endValue += 0.07;
	    	beginValue = (double) Math.round(beginValue * 100) / 100;
	    	endValue = (double) Math.round(endValue * 100) / 100;

	    	output = "<flow id=\"r5_20060"
	    			+ (i / 24 + 6)
	    			+ "_"
	    			+ (i % 24 + 1)
	    			+ "\" begin=\""
	    			+ beginValue
	    			+ "\" end=\""
	    			+ endValue
	    			+ "\" vehsPerHour=\""
	    			+ accumulateVechiclesVector[0][i]
	    			+ "\" type=\"pkw_ohne_anhaenger\" route=\"route5\"/>";
	    	writer.append(TAB_DELIMITER)
	    		  .append(output)
	    		  .append("\n");
	    	
	    	
	    	beginValue += 0.07;
	    	endValue += 0.07;
	    	beginValue = (double) Math.round(beginValue * 100) / 100;
	    	endValue = (double) Math.round(endValue * 100) / 100;
	    	
	    	output = "<flow id=\"r7_20060"
	    			+ (i / 24 + 6)
	    			+ "_"
	    			+ (i % 24 + 1)
	    			+ "\" begin=\""
	    			+ beginValue
	    			+ "\" end=\""
	    			+ endValue
	    			+ "\" vehsPerHour=\""
	    			+ accumulateVechiclesVector[1][i]
	    			+ "\" type=\"pkw_ohne_anhaenger\" route=\"route7\"/>";
	    	writer.append(TAB_DELIMITER)
  		  		  .append(output)
  		  		  .append("\n");
	    	
	    	
	    	beginValue += 0.07;
	    	endValue += 0.07;
	    	beginValue = (double) Math.round(beginValue * 100) / 100;
	    	endValue = (double) Math.round(endValue * 100) / 100;
	    	
	    	output = "<flow id=\"r9_20060"
	    			+ (i / 24 + 6)
	    			+ "_"
	    			+ (i % 24 + 1)
	    			+ "\" begin=\""
	    			+ beginValue
	    			+ "\" end=\""
	    			+ endValue
	    			+ "\" vehsPerHour=\""
	    			+ accumulateVechiclesVector[2][i]
	    			+ "\" type=\"pkw_ohne_anhaenger\" route=\"route9\"/>";
	    	writer.append(TAB_DELIMITER)
  		  		  .append(output)
  		  		  .append("\n");
	    	
	    	//Separate hours
	    	writer.append("\n");
	    	
	    	//Separate days
	    	if(i%24 == 23)
	    		writer.append("\n\n");
	    }
	    
	    writer.close();
        System.out.println("Done!");
	}

	
	private static int getMaxNumberBetweenTwo(int number1, int number2) {
		return number1 > number2 ? number1 : number2;
	}
}

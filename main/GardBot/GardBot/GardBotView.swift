import SwiftUI
import WebKit

struct ServerResponse: Codable 
{
    let date: String
    let time: String
    let oat: Double
    let oah: Double
    let sm: Double
    let st: Double
    let sec: Double
    let sph: Double
    let watering: Bool
    let wateringTimeRemaining: Int

    enum CodingKeys: String, CodingKey 
    {
        case date = "Date"
        case time = "Time"
        case oat = "OAT"
        case oah = "OAH"
        case sm = "SM"
        case st = "ST"
        case sec = "SEC"
        case sph = "SPH"
        case watering = "WATERING"
        case wateringTimeRemaining = "WATERINGTIMEREMAINING"
    }
}

struct GardBotView: View 
{
    @State private var serverResponse: ServerResponse?
    @State var isWaterCycleActive = false
    @State private var isToggleDisabled = false
    @State private var isToggleDisabledlast = false

    let timer = Timer.publish(every: 2, on: .main, in: .common).autoconnect()

    var body: some View 
    {
        GeometryReader { geometry in
            VStack 
            {
                Toggle(isOn: $isWaterCycleActive) {
                    Text("Activate Water Cycle")
                }
                .padding(.horizontal, 60.0)
                .onChange(of: isWaterCycleActive) { newValue in
                    isToggleDisabled = true
                    DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                        sendTag(tag: newValue ? "StartWater" : "StopWater")
                    }
                    DispatchQueue.main.asyncAfter(deadline: .now() + 5) {
                        isToggleDisabled = false
                    }
                }
                if isToggleDisabled
                {
                    ProgressView()
                        .progressViewStyle(CircularProgressViewStyle())
                } 

                if let response = serverResponse 
                {
                    HStack {
                        Text("Watering Cycle Active: ")
                        Image(systemName: response.watering ? "checkmark.circle.fill" : "xmark.circle.fill")
                            .resizable()
                            .frame(width: 24, height: 24)
                            .foregroundColor(response.watering ? .green : .red)
                    }
                    .padding()
                    // Display the SM "soil moisture" value
                    HStack {
                        Text("Soil Moisture: ")
                        Text(String(format: "%.1f", response.sm))
                    }
                    // Display the ST "soil temperature" value
                    HStack {
                        Text("Soil Temp: ")
                        Text(String(format: "%.0f", response.st))
                    }
                    // Display the SPH "soil pH" value
                    HStack {
                        Text("Soil pH: ")
                        Text(String(format: "%.1f", response.sph))
                    }
                    // Display watering time remaining
                    HStack {
                        Text("Watering Time Remaining: ")
                        Text("\(response.wateringTimeRemaining)")
                    }
                    //display Server Time
                    HStack {
                        Text("Server Time: ")
                        Text("\(response.date) \(response.time)")
                    }
                }
            }
        }
        .onReceive(timer) 
        { _ in
            // Reload the WebView every time the timer fires
            sendTag(tag: "Refresh")
        }
    }

    func sendTag(tag: String) 
    {
        guard let url = URL(string: "http://192.168.0.91/\(tag)") else
        {
            print("Invalid URL")
            return
        }

            let task = URLSession.shared.dataTask(with: url)
            { (data, response, error) in
                if let error = error
                {
                    print("HTTP Request Failed \(error)")
                }
                else if let data = data
                {
                    if(tag == "Refresh")
                    {
                        if( isToggleDisabled == false || isToggleDisabledlast)
                        {
                            let decoder = JSONDecoder()
                            do
                            {
                                let response = try decoder.decode(ServerResponse.self, from: data)
                                print("Server Response: \(response)")
                                serverResponse = response
                                isToggleDisabled = false;
                                isToggleDisabledlast = false;
                            }
                            catch
                            {
                                print("Failed to decode JSON: \(error)")
                            }
                        }
                        else
                        {
                            isToggleDisabledlast = isToggleDisabled
                            sendTag(tag: "Refresh");
                        }
                    }
                }
            }
            task.resume()
        
    }
}

// PreviewProvider for GardBotView
struct GardBotView_Previews: PreviewProvider 
{
    static var previews: some View 
    {
        GardBotView()
    }
}


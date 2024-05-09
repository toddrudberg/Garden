import SwiftUI
import WebKit

struct ServerResponse: Codable 
{
    let date: String
    let time: String
    let oat: Float
    let oah: Float
    let sm: Float
    let st: Float
    let sec: Float
    let sph: Float
    let watering: Bool
    let wateringTimeRemaining: Float

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
    @State var toggleWaterCycleActiveOn = false
    @State private var isToggleDisabled = false
    @State private var fontSize1 = 30
    @State private var processStep = 0
    @State private var isInitializing = true

    let timer = Timer.publish(every: 2, on: .main, in: .common).autoconnect()

    var body: some View 
    {
        GeometryReader { geometry in
            VStack 
            {
                Toggle(isOn: $toggleWaterCycleActiveOn) 
                {
                    Text("Activate Cycle")
                        .font(.system(size: CGFloat(fontSize1)))
                }
                .padding([.top, .leading, .trailing], 60.0)
                .onChange(of: toggleWaterCycleActiveOn) 
                    { newValue in
                        isToggleDisabled = true
                        processStep = 1
                    }
                .disabled(isToggleDisabled || isInitializing)
                ProgressView()
                    .progressViewStyle(CircularProgressViewStyle())
                    .opacity(isToggleDisabled || isInitializing ? 1 : 0)
                if let response = serverResponse
                {
                    HStack {
                        Text("Watering Cycle Active: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Image(systemName: response.watering ? "checkmark.circle.fill" : "xmark.circle.fill")
                            .resizable()
                            .frame(width: 24, height: 24)
                            .foregroundColor(response.watering ? .green : .red)
                    }
                    .padding()
                    // Display the SM "soil moisture" value
                    HStack {
                        Text("Soil Moisture: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Text(String(format: "%.1f", response.sm))
                            .font(.system(size: CGFloat(fontSize1)))
                    }
                    // Display the ST "soil temperature" value
                    HStack {
                        Text("Soil Temp: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Text(String(format: "%.0f", response.st))
                            .font(.system(size: CGFloat(fontSize1)))
                    }
                    // Display the SPH "soil pH" value
                    HStack {
                        Text("Soil pH: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Text(String(format: "%.1f", response.sph))
                            .font(.system(size: CGFloat(fontSize1)))
                    }
                    // Display watering time remaining
                    HStack {
                        Text("Time Remaining: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Text(String(format: "%02d:%02d",
                                    Int(response.wateringTimeRemaining),
                                    Int((response.wateringTimeRemaining * 60).truncatingRemainder(dividingBy: 60))))
                            .font(.system(size: CGFloat(fontSize1)))
                    }
                    //display Server Date
                    HStack {
                        Text("Server Date: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Text("\(response.date)")
                            .font(.system(size: CGFloat(fontSize1)))
                    }
                    //display Server Time
                    HStack {
                        Text("Server Time: ")
                            .font(.system(size: CGFloat(fontSize1)))
                        Text("\(response.time)")
                            .font(.system(size: CGFloat(fontSize1)))
                    }
                }
            }
        }
        .onReceive(timer) 
        { _ in

            switch processStep 
            {
                case 0:
                    sendTag(tag: "Refresh")
                    isInitializing = false
                    break
                case 1:
                    if( toggleWaterCycleActiveOn == true)
                    {
                        sendTag(tag: "StartWater")
                    }
                    else
                    {
                        sendTag(tag: "StopWater")
                    }
                    processStep += 1
                    break
                case 2:
                    sendTag(tag: "Refresh")
                    processStep = 0
                    isToggleDisabled = false
                    break
                default:
                    break
            }
        }
        .background(
            Image("Background-1")
                .resizable()
                .aspectRatio(contentMode: .fill)
                .edgesIgnoringSafeArea(.all))
    }

    func sendTag(tag: String) 
    {
        guard let url = URL(string: "http://192.168.1.9/\(tag)") else
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
                    let decoder = JSONDecoder()
                    do
                    {
                        let response = try decoder.decode(ServerResponse.self, from: data)
                        print("Server Response: \(response)")
                        serverResponse = response
                    }
                    catch
                    {
                        print("Failed to decode JSON: \(error)")
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

